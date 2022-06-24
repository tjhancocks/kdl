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

#include <utility>
#include <iterator>
#include <iostream>
#include "diagnostic/fatal.hpp"
#include "parser/sema/declarations/named_types/file_type_parser.hpp"
#include "media/conversion.hpp"
#include "media/sprite_sheet_assembler.hpp"
#include "parser/file.hpp"

// MARK: - Constructor

kdl::sema::file_type_parser::file_type_parser(kdl::sema::parser &parser, kdl::build_target::type_field &field,
                                              kdl::build_target::type_field_value &field_value,
                                              kdl::build_target::type_template::binary_field binary_field,
                                              kdl::build_target::kdl_type &type,
                                              std::weak_ptr<kdl::target> target)
    : m_parser(parser),
      m_explicit_type(type),
      m_field(field),
      m_binary_field(std::move(binary_field)),
      m_field_value(field_value),
      m_target(std::move(target))
{

}

// MARK: - Parser

auto kdl::sema::file_type_parser::parse(kdl::build_target::resource_instance &instance) -> void
{
    std::vector<lexeme> file_lx;
    std::vector<graphite::data::block> file_contents;
    auto target = m_target.lock();
    auto import_file = false;

    if (m_parser.expect({ expectation(lexeme::identifier, "import").be_true() })) {
        m_parser.advance();
        import_file = true;
    }

    auto string_to_vector = [](const std::string& s) -> std::vector<char> {
        std::vector<char> v;
        std::copy(s.begin(), s.end(), std::back_inserter(v));
        return v;
    };

    auto vector_to_string = [](const std::vector<char>& v) -> std::string {
        std::string s;
        std::copy(v.begin(), v.end(), std::back_inserter(s));
        return s;
    };

    auto block_to_string = [&](const graphite::data::block& block) -> std::string {
        graphite::data::reader reader(&block);
        return vector_to_string(std::move(reader.read_bytes(reader.size())));
    };

    // Build a list of file contents, or file paths if import was specified
    while (m_parser.expect({ expectation(lexeme::string).be_true() })) {
        auto string_lx = m_parser.read();
        auto content_value = string_to_vector(string_lx.text());

        if (import_file) {
            // Pass the resolved paths through glob to expand wildcards
            auto path = target->resolve_src_path(string_lx.text());
            auto paths = file::glob(path);

            for (const auto& p : *paths) {
                content_value = kdl::file(p).vector();
                file_lx.emplace_back(lexeme(p, lexeme::string));
                file_contents.emplace_back(content_value);
            }
        }
        else {
            file_lx.emplace_back(string_lx);
            file_contents.emplace_back(content_value);
        }
    }

    if (file_contents.empty()) {
        log::fatal_error(m_parser.peek(), 1, "Fields with the 'File' type expect a string.");
    }

    auto string_lx = file_lx.back();
    auto content_value = file_contents.back();

    // Check if we need to perform a conversion on the file data.
    if (m_field_value.has_conversion_defined()) {
        // Get the defined input format.
        std::vector<lexeme> valid_input_formats;
        if (m_field_value.conversion_input().is(lexeme::var, "InputFormat")) {
            // The conversion is expecting the File to carry it's own input type.
            valid_input_formats = m_explicit_type.type_hints();
        }
        else {
            // The conversion has specified exactly what is going to happen. The file type hints will be ignored.
            valid_input_formats.emplace_back(m_field_value.conversion_input());
        }

        // For now we expect only a single input type.
        if (valid_input_formats.empty() || valid_input_formats.size() > 1) {
            log::fatal_error(m_field_value.conversion_input(), 1, "Bad conversion map. Unable to deduce input format.");
        }

        // Perform the conversion
        auto input_format = valid_input_formats.at(0);
        auto output_format = m_field_value.conversion_output();

        if (file_contents.size() != 1) {
            auto conversion = kdl::media::conversion(input_format, output_format);
            for (const auto& f : file_contents) {
                conversion.add_input_data(f);
            }
            content_value = std::move(conversion.perform_conversion());
        }
        else {
            content_value = kdl::media::conversion(content_value, input_format, output_format).perform_conversion();
        }
    }

    // Check if we're assembling a sprite sheet (this involves taking multiple input files and putting them
    // into a single image and export it as TGA data)
    else if (m_field_value.assemble_sprite_sheet()) {
        content_value = kdl::media::sprite_sheet_assembler(file_contents, m_explicit_type.type_hints()[0]).assemble();
    }

    // Get the value type for the field, and the set it.
    switch (m_binary_field.type & ~0xFFFUL) {
        case build_target::PSTR: {
            if (content_value.size() > 255) {
                log::fatal_error(string_lx, 1, "String too large for value type.");
            }
            instance.write_pstr(m_field, m_field_value, block_to_string(content_value), 0);
            break;
        }
        case build_target::CSTR: {
            instance.write_cstr(m_field, m_field_value, block_to_string(content_value));
            break;
        }
        case build_target::Cnnn: {
            auto size = static_cast<std::size_t>(m_binary_field.type) & 0xFFFUL;
            if (content_value.size() > size) {
                log::fatal_error(string_lx, 1, "String too large for value type.");
            }
            instance.write_cstr(m_field, m_field_value, block_to_string(content_value), size);
            break;
        }
//        case build_target::P0nn: {
//            auto size = static_cast<std::size_t>(m_binary_field.type) & 0x0FF;
//            if (string_value.size() > size) {
//                log::fatal_error(string_lx, 1, "String too large for value type.");
//            }
//            instance.write_pstr(m_field, m_field_value, string_value, size);
//            break;
//        }
        case build_target::HEXD: {
            instance.write_data(m_field, m_field_value, content_value);
            break;
        }
        default: {
            log::fatal_error(string_lx, 1, "Unsupported value type for field '" + m_field.name().text() + "' with a type 'File'.");
        }
    }
}
