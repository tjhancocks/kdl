// Copyright (c) 2020 Tom Hancocks
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <map>
#include <fstream>
#include "disassembler/resource_exporter.hpp"
#include "libGraphite/data/reader.hpp"
#include "media/conversion.hpp"

// MARK: - Construction

kdl::disassembler::resource_exporter::resource_exporter(kdl::disassembler::kdl_exporter &exporter,
                                                        build_target::type_container& type)
    : m_exporter(exporter), m_container(type)
{

}

// MARK: - Disassembler

static auto extract_value(graphite::data::reader& r, kdl::build_target::type_template& tmpl, int& index, std::vector<std::any>& disassembled_values) -> void
{
    auto tmpl_field = tmpl.binary_field_at(index);

    switch (tmpl_field.type & ~0xFFF) {
        case kdl::build_target::HBYT: {
            disassembled_values.emplace_back(r.read_byte());
            break;
        }
        case kdl::build_target::HWRD: {
            disassembled_values.emplace_back(r.read_short());
            break;
        }
        case kdl::build_target::HLNG: {
            disassembled_values.emplace_back(r.read_long());
            break;
        }
        case kdl::build_target::HQAD: {
            disassembled_values.emplace_back(r.read_quad());
            break;
        }
        case kdl::build_target::DBYT: {
            disassembled_values.emplace_back(r.read_signed_byte());
            break;
        }
        case kdl::build_target::DWRD: {
            disassembled_values.emplace_back(r.read_signed_short());
            break;
        }
        case kdl::build_target::DLNG: {
            disassembled_values.emplace_back(r.read_signed_long());
            break;
        }
        case kdl::build_target::DQAD: {
            disassembled_values.emplace_back(r.read_signed_quad());
            break;
        }
        case kdl::build_target::PSTR: {
            disassembled_values.emplace_back(r.read_pstr());
            break;
        }
        case kdl::build_target::CSTR: {
            disassembled_values.emplace_back(r.read_cstr());
            break;
        }
        case kdl::build_target::Cnnn: {
            auto length = tmpl_field.type & 0xFFF;
            disassembled_values.emplace_back(r.read_cstr(length));
            break;
        }
        case kdl::build_target::RECT: {
            disassembled_values.emplace_back(std::tuple<int16_t, int16_t, int16_t, int16_t>(
                r.read_signed_short(), r.read_signed_short(), r.read_signed_short(), r.read_signed_short()
            ));
            break;
        }
        case kdl::build_target::HEXD: {
            disassembled_values.emplace_back(r.read_bytes(r.size() - r.position()));
            break;
        }
        case kdl::build_target::OCNT: {
            auto count = r.read_short();
            disassembled_values.emplace_back(count);
            std::vector<std::any> items;

            ++index;
            if (count != 0) {
                for (auto n = 0; n < count; ++n) {
                    extract_value(r, tmpl, index, items);
                }
            }

            disassembled_values.emplace_back(items);
        }
    }
}

auto kdl::disassembler::resource_exporter::disassemble(std::shared_ptr<graphite::rsrc::resource> resource) -> void
{
    // The first task is to read all values out of the resource fork. This will make it easier to handle
    // merging and splitting of values required by fields.
    graphite::data::reader r(resource->data());
    std::vector<std::any> disassembled_values;
    auto tmpl = m_container.internal_template();
    for (auto i = 0; i < tmpl.binary_field_count(); ++i) {
        extract_value(r, tmpl, i, disassembled_values);
    }

    // We now have a list of all the values for the resource, iterate over the fields, and work out what needs to be
    // done.
    for (auto field : m_container.all_fields()) {

        // Iterate over all field repetitions (even if its just a single field)
        for (auto n = field.lower_repeat_bound(); n <= field.upper_repeat_bound(); ++n) {

            // Configure a set of repetition expansion variables.
            std::map<std::string, lexeme> vars {
                std::make_pair("FieldNumber", lexeme(std::to_string(n), lexeme::integer))
            };

            // Now loop over the expected values.
            std::vector<std::string> formatted_values;
            for (auto m = 0; m < field.expected_values(); ++m) {
                auto expected_value = field.value_at(m);
                auto expanded_name = expected_value.extended_name(vars);

                auto tmpl_field_index = tmpl.binary_field_index(expanded_name);
                auto disasm_value = disassembled_values.at(tmpl_field_index);
                auto tmpl_field = tmpl.binary_field_at(tmpl_field_index);

                // Check if the prior binary field is of type OCNT. If it is then we are dealing with a counted
                // list.
                // TODO: This is very hacky and needs to be cleaned up.
                if (tmpl_field_index > 0 && tmpl.binary_field_at(tmpl_field_index - 1).type == build_target::OCNT) {
                    // For now we're going to use the implicit value here (as STR# is the main resource using this)
                    for (auto v : std::any_cast<std::vector<std::any>>(disasm_value)) {
                        auto v_str = disassemble_value(tmpl_field.type, expected_value, v);
                        m_exporter.add_field(field.name().text(), { v_str });
                    }
                    return;
                }

                // Check if the field has a conversion applied for it. If it does then ask the conversion
                // module to perform the conversion.
                if (expected_value.has_conversion_defined()) {
                    media::conversion conv(expected_value.conversion_output(), lexeme("PNG", lexeme::identifier));
                    conv.add_input_data(std::any_cast<std::vector<char>>(disasm_value));
                    auto data = conv.perform_conversion();

                    auto file_name = m_container.name() + "-" + std::to_string(resource->id()) + ".png";
                    m_exporter.export_file(file_name, data);
                }

                if (expected_value.explicit_type().has_value()) {
                    // We've got an explicit type supplied and thus need to do something.
                    if (expected_value.explicit_type()->is_reference()) {
                        if (auto id = std::any_cast<int16_t>(disasm_value)) {
                            formatted_values.emplace_back("#" + std::to_string(id));
                        }
                    }
                    else if (expected_value.explicit_type()->name()->is("File")) {
                        auto file_name = m_container.name() + "-" + std::to_string(resource->id()) + ".txt";
                        if (expected_value.explicit_type()->type_hints().empty()) {
                            m_exporter.export_file(file_name, std::any_cast<std::string>(disasm_value));
                        }
                        formatted_values.emplace_back("import \"" + file_name + "\"");
                    }
                    else {
                        auto v_str = disassemble_value(tmpl_field.type, expected_value, disasm_value);
                        formatted_values.emplace_back(v_str);
                    }
                }
                else {
                    auto v_str = disassemble_value(tmpl_field.type, expected_value, disasm_value);
                    formatted_values.emplace_back(v_str);
                }
            }

            // Add the field into the exporter.
            m_exporter.add_field(field.name().text(), formatted_values);
        }
    }
}

auto kdl::disassembler::resource_exporter::disassemble_value(kdl::build_target::binary_type base_type,
                                                             kdl::build_target::type_field_value &expected_value,
                                                             std::any disasm_value) const -> std::string
{
    switch (base_type & ~0xFFF) {
        case build_target::CSTR:
        case build_target::Cnnn:
        case build_target::PSTR: {
            return "\"" + std::any_cast<std::string>(disasm_value) + "\"";
        }
        case build_target::RECT: {
            auto rect = std::any_cast<std::tuple<int16_t, int16_t, int16_t, int16_t>>(disasm_value);
            return std::to_string(std::get<0>(rect)) + " "
                 + std::to_string(std::get<0>(rect)) + " "
                 + std::to_string(std::get<0>(rect)) + " "
                 + std::to_string(std::get<0>(rect));
        }
        case build_target::HBYT: {
            return std::to_string(std::any_cast<uint8_t>(disasm_value));
        }
        case build_target::DBYT: {
            return std::to_string(std::any_cast<int8_t>(disasm_value));
        }
        case build_target::HWRD: {
            return std::to_string(std::any_cast<uint16_t>(disasm_value));
        }
        case build_target::DWRD: {
            return std::to_string(std::any_cast<int16_t>(disasm_value));
        }
        case build_target::HLNG: {
            return std::to_string(std::any_cast<uint32_t>(disasm_value));
        }
        case build_target::DLNG: {
            return std::to_string(std::any_cast<int32_t>(disasm_value));
        }
        case build_target::HQAD: {
            return std::to_string(std::any_cast<uint64_t>(disasm_value));
        }
        case build_target::DQAD: {
            return std::to_string(std::any_cast<int64_t>(disasm_value));
        }
    }

    return "??";
}
