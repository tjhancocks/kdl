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
#include <algorithm>
#include <stdexcept>
#include <climits>
#include "disassembler/binary_parser.hpp"
#include "disassembler/resource_exporter.hpp"
#include <libGraphite/data/reader.hpp>
#include "media/conversion.hpp"

// MARK: - Construction

kdl::disassembler::resource_exporter::resource_exporter(task& task, kdl::disassembler::kdl_exporter &exporter,
                                                        build_target::type_container& type)
    : m_exporter(exporter),
      m_container(type),
      m_task(task),
      m_id( std::numeric_limits<graphite::rsrc::resource::identifier>::max())
{
}

// MARK: - Extraction

auto kdl::disassembler::resource_exporter::extract_kdl_field(const build_target::type_field& field, int pass) -> void
{
    // Field names can be "expanded" so we need to account for this... This is actually exposed in the
    // "type_field_value" as a name expansion.
    std::unordered_map<std::string, lexeme> expansion_vars {
        std::make_pair("FieldNumber", lexeme(std::to_string(pass), lexeme::integer))
    };

    // The next step is to extract the appropriate values from the binary resource. These have already been parsed
    // into a std::map that we can access, but we still need to decode the type for use here.
    // For this we can iterate through each of the expected values for this field. This will tell us exactly which
    // binary field offset to find the appropriate value.
    for (auto n = 0; n < field.expected_values(); ++n) {
        const auto& expected_value = field.value_at(n);
        const auto& expanded_name = expected_value.extended_name(expansion_vars);

        // We need to know exactly which binary field in the template we're dealing with for this value. From that
        // we can then get the extracted value from the binary resource.
        auto tmpl_field_index = m_container.internal_template().binary_field_index(expanded_name);
        const auto& tmpl_field_info = m_container.internal_template().binary_field_at(tmpl_field_index);
        const auto& extracted_value = m_extracted_values.at(tmpl_field_index);

        // List of repeated items.
        if (tmpl_field_index > 0 && m_container.internal_template().binary_field_at(tmpl_field_index - 1).type == build_target::OCNT) {
            auto ocnt = m_container.internal_template().binary_field_at(tmpl_field_index - 1);
            auto items = std::any_cast<std::vector<std::any>>(extracted_value);
            std::vector<std::string> strings;

            for (auto item : items) {
                strings.emplace_back(write_field_value(tmpl_field_info, item));
            }

            m_final_field_repeat.emplace(tmpl_field_index, strings);
            m_final_values.emplace(tmpl_field_index, "<REPEATED>");
            return;
        }

        // Check if the template field has been visited already. If it has check if this value has a higher
        // priority than the previous. If it does then replace the previous visitation with this one.
        auto priority = std::numeric_limits<std::int32_t>::max(); // Default the "higest value"/"lowest priority".
        auto is_file = false;
        if (expected_value.explicit_type().has_value() && expected_value.explicit_type()->name()->is("File")) {
            // The type hints here denote a file format. If no type hints exist, then use the default priority above.
            is_file = true;
            const auto& file_type_hints = expected_value.explicit_type()->type_hints();
            if (!file_type_hints.empty()) {
                // Only take the first type hint. The others are "undefined" behaviour.
                priority = m_task.format_priority(file_type_hints.at(0));
            }
        }

        if (m_visited_template_fields.find(tmpl_field_index) != m_visited_template_fields.end()) {
            // We have visited this template field before, but does this priority beat the existing one? If the new
            // priority is the _same_ or greater than the existing one, the ignore it.
            if (m_visited_template_fields.at(tmpl_field_index) > priority) {
                return;
            }
        }

        // Update the visitation information, and then proceed to handle the value of the field.
        m_visited_template_fields.emplace(tmpl_field_index, priority);
        m_final_field_assoc.emplace(tmpl_field_index, field.name().text());

        // Check if there is a conversion pass assigned to the value. If there is, then we need to take the binary
        // content, and run it through the conversion in reverse.
        if (expected_value.has_conversion_defined() && is_file) {
            const auto output_format = m_task.appropriate_conversion_format(expected_value.conversion_output(), priority);
            if (!output_format.has_value()) {
                continue;
            }

            media::conversion conv(expected_value.conversion_output(), output_format.value());
            conv.add_input_data(std::any_cast<std::vector<char>>(extracted_value));
            auto result = conv.perform_conversion();

            // Now that the conversion has been defined setup a file export for this data. We can't immediately save
            // this data though, as this may not be the final result.
            std::string export_path(m_container.name() + "-" + std::to_string(m_id) + "." + m_task.format_extension(output_format.value()));
            m_file_exports.emplace(tmpl_field_index, std::make_tuple(std::move(result), export_path));
            m_final_values.emplace(tmpl_field_index, "import \"" + export_path + "\"");
            return;
        }
        else if (is_file) {
            // Setup a file export for this data. We can't immediately save this data though, as this may not be the
            // final result.
            auto content = std::any_cast<std::string>(extracted_value);
            std::string export_path(m_container.name() + "-" + std::to_string(m_id) + ".txt");
            m_file_exports.emplace(tmpl_field_index, std::make_tuple(std::vector<char>(content.begin(), content.end()), export_path));
            m_final_values.emplace(tmpl_field_index, "import \"" + export_path + "\"");
            return;
        }

        // Before proceeding now, we need to check for any potential substitution symbols that could represent the value
        const auto substitutions = find_substitutions(field, extracted_value);
        if (!substitutions.empty()) {
            // We have identified one or more substituions. Only Bitmask types can have more than one substitution value
            // though.
            if (expected_value.explicit_type().has_value() && expected_value.explicit_type()->name()->is("Bitmask")) {
                std::string mask;
                for (auto lx = substitutions.begin(); lx != substitutions.end(); ++lx) {
                    if (lx != substitutions.begin()) {
                        mask.append(" | ");
                    }
                    mask.append(lx->text());
                }
                m_final_values.emplace(tmpl_field_index, mask);
            }
            else {
                m_final_values.emplace(tmpl_field_index, substitutions.at(0).text());
            }
            return;
        }

        if (expected_value.explicit_type().has_value() && expected_value.explicit_type()->name()->is("Color")) {
            auto color = std::any_cast<uint32_t>(extracted_value);
            auto r = static_cast<uint8_t>((color >> 16) & 0xFF);
            auto g = static_cast<uint8_t>((color >> 8) & 0xFF);
            auto b = static_cast<uint8_t>((color) & 0xFF);
            m_final_values.emplace(tmpl_field_index, "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")");
            continue;
        }

        // There is no conversion applied, so we need to interpret the value literally.
        if (expected_value.explicit_type().has_value() && expected_value.explicit_type()->is_reference()) {
            // We're looking at a resource id - extract the value as an integer.
            try {
                auto id = std::any_cast<int16_t>(extracted_value);
                m_final_values.emplace(tmpl_field_index, "#" + std::to_string(id));
            }
            catch (std::bad_any_cast& e) {
                try {
                    auto id = std::any_cast<int64_t>(extracted_value);
                    m_final_values.emplace(tmpl_field_index, "#" + std::to_string(id));
                }
                catch (std::bad_any_cast& e) {
                    throw std::logic_error("Bad resource id type encountered");
                }
            }

            continue;
        }

        // In to correct render the value, we need to check what its actual type is.
        m_final_values.emplace(tmpl_field_index, write_field_value(tmpl_field_info, extracted_value));
    }
}

auto kdl::disassembler::resource_exporter::write_field_value(const build_target::type_template::binary_field& tmpl_field_info,
                                                             const std::any& extracted_value) -> std::string
{
    switch (tmpl_field_info.type & ~0xFFF) {
        case build_target::DBYT: {
            return std::to_string(std::any_cast<int8_t>(extracted_value));
        }
        case build_target::DWRD: {
            return std::to_string(std::any_cast<int16_t>(extracted_value));
        }
        case build_target::DLNG: {
            return std::to_string(std::any_cast<int32_t>(extracted_value));
        }
        case build_target::DQAD: {
            return std::to_string(std::any_cast<int64_t>(extracted_value));
        }
        case build_target::HBYT: {
            char out[5] = { 0 };
            sprintf(out, "0x%02x", std::any_cast<uint8_t>(extracted_value));
            return std::string(out);
        }
        case build_target::HWRD: {
            char out[7] = { 0 };
            sprintf(out, "0x%04x", std::any_cast<uint16_t>(extracted_value));
            return std::string(out);
        }
        case build_target::HLNG: {
            char out[11] = { 0 };
            sprintf(out, "0x%08x", std::any_cast<uint32_t>(extracted_value));
            return std::string(out);
        }
        case build_target::HQAD: {
            char out[19] = { 0 };
            sprintf(out, "0x%016llx", std::any_cast<uint64_t>(extracted_value));
            return std::string(out);
        }
        case build_target::CSTR:
        case build_target::Cnnn:
        case build_target::PSTR: {
            return std::string("\"" + escape_strings(std::any_cast<std::string>(extracted_value)) + "\"");
        }
        case build_target::RECT: {
            auto rect = std::any_cast<std::tuple<int16_t, int16_t, int16_t, int16_t>>(extracted_value);
            std::string rect_string;
            rect_string.append(std::to_string(std::get<0>(rect)) + " ");
            rect_string.append(std::to_string(std::get<0>(rect)) + " ");
            rect_string.append(std::to_string(std::get<0>(rect)) + " ");
            rect_string.append(std::to_string(std::get<0>(rect)));
            return rect_string;
        }
        default: {
            return "0";
        }
    }
}

auto kdl::disassembler::resource_exporter::repeat_kdl_field_extraction(const build_target::type_field& field) -> void
{
    if (field.is_repeatable() && (field.upper_repeat_bound() - field.lower_repeat_bound()) <= m_extracted_values.size()) {
        for (auto pass = field.lower_repeat_bound(); pass <= field.upper_repeat_bound(); ++pass) {
            extract_kdl_field(field, pass);
        }
    }
    else {
        extract_kdl_field(field);
    }
}

// MARK: - Exporting

auto kdl::disassembler::resource_exporter::export_kdl_field(const kdl::build_target::type_field &field, int pass) -> void
{
    // Field names can be "expanded" so we need to account for this... This is actually exposed in the
    // "type_field_value" as a name expansion.
    std::unordered_map<std::string, lexeme> expansion_vars {
        std::make_pair("FieldNumber", lexeme(std::to_string(pass), lexeme::integer))
    };

    // The next step is to extract the appropriate values from the binary resource. These have already been parsed
    // into a std::map that we can access, but we still need to decode the type for use here.
    // For this we can iterate through each of the expected values for this field. This will tell us exactly which
    // binary field offset to find the appropriate value.
    std::vector<std::string> values;
    for (auto n = 0; n < field.expected_values(); ++n) {
        const auto &expected_value = field.value_at(n);
        const auto &expanded_name = expected_value.extended_name(expansion_vars);

        // We need to know exactly which binary field in the template we're dealing with for this value. From that
        // we can then get the extracted value from the binary resource.
        auto tmpl_field_index = m_container.internal_template().binary_field_index(expanded_name);
        if (m_final_values.find(tmpl_field_index) != m_final_values.end() && m_final_values.at(tmpl_field_index) == "<REPEATED>") {
            for (auto value : m_final_field_repeat.at(tmpl_field_index)) {
                m_exporter.add_field(field.name().text(), { value });
            }
            return;
        }

        if (m_final_field_assoc.find(tmpl_field_index) != m_final_field_assoc.end() && m_final_field_assoc.at(tmpl_field_index) != field.name().text()) {
            return;
        }

        values.emplace_back(m_final_values.at(tmpl_field_index));
    }

    // Export the field.
    m_exporter.add_field(field.name().text(), values);
}

auto kdl::disassembler::resource_exporter::repeat_kdl_field_export(const build_target::type_field& field) -> void
{
    if (field.is_repeatable() && (field.upper_repeat_bound() - field.lower_repeat_bound()) <= m_extracted_values.size()) {
        for (auto pass = field.lower_repeat_bound(); pass <= field.upper_repeat_bound(); ++pass) {
            export_kdl_field(field, pass);
        }
    }
    else {
        export_kdl_field(field);
    }
}

// MARK: - Disassembler

auto kdl::disassembler::resource_exporter::disassemble(graphite::rsrc::resource& resource) -> void
{
    m_id = resource.id();

    // The first task is to read all values out of the resource fork. This will make it easier to handle
    // merging and splitting of values required by fields.
    graphite::data::reader r(&resource.data());
    m_extracted_values = binary_parser(m_container.internal_template()).parse(r);

    // We now have a list of all the values for the resource, iterate over the fields, and work out what needs to be
    // done.
    for (const auto& field : m_container.all_fields()) {
        repeat_kdl_field_extraction(field);
    }

    // Now that all values have been extracted and formatted, do a second pass and actually build the output
    for (const auto& field : m_container.all_fields()) {
        repeat_kdl_field_export(field);
    }

    // Perform any file extractions.
    for (auto extraction : m_file_exports) {
        m_exporter.export_file(std::get<1>(extraction.second), std::get<0>(extraction.second));
    }
}

// MARK: - Helpers

auto kdl::disassembler::resource_exporter::find_substitutions(const kdl::build_target::type_field &field,
                                                              const std::any &value) const -> std::vector<lexeme>
{
    std::vector<lexeme> subs;
    return subs;
}
