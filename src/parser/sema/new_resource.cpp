// Copyright (c) 2019-2020 Tom Hancocks
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

#include "libGraphite/data/writer.hpp"
#include "diagnostic/fatal.hpp"
#include "parser/sema/new_resource.hpp"

auto kdl::sema::new_resource::test(kdl::sema::parser &parser) -> bool
{
    return parser.expect({
        expectation(lexeme::identifier, "new").be_true()
    });
}

auto kdl::sema::new_resource::parse(kdl::sema::parser &parser, kdl::container &type_container, std::weak_ptr<kdl::target> target) -> void
{
    // Begin a new resource instance, and get the ID and Name of the resource if provided.
    parser.ensure({
        expectation(lexeme::identifier, "new").be_true(),
        expectation(lexeme::l_paren).be_true()
    });

    int64_t resource_id = INT64_MIN;
    std::string resource_name;

    if (parser.expect({
        expectation(lexeme::res_id).be_true(),
        expectation(lexeme::comma).be_true(),
        expectation(lexeme::string).be_true()
    })) {
        resource_id = std::stoll(parser.read().text());
        parser.advance();
        resource_name = parser.read().text();
    }
    else if (parser.expect({
        expectation(lexeme::res_id).be_true()
    })) {
        resource_id = parser.read().value<decltype(resource_id)>();
    }
    else if (parser.expect({
        expectation(lexeme::string).be_true()
    })) {
        resource_name = parser.read().text();
    }

    parser.ensure({ expectation(lexeme::r_paren).be_true() });

    // Begin the resource declaration block.
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer resource_data(data);

    parser.ensure({ expectation(lexeme::l_brace).be_true() });

    while (parser.expect({ expectation(lexeme::r_brace).be_false() })) {

        // First we expect a field name.
        if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
            auto lx = parser.peek();
            log::fatal_error(lx, 1, "Expected an identifier for the field name.");
        }
        auto field_name = parser.read();
        auto field = type_container.field_named(field_name);

        parser.ensure({ expectation(lexeme::equals).be_true() });

        // Check if an explicit type was given for the field.
        if (field.type().has_value()) {
            auto type = field.type().value();

            // Check if the explict type is a reference type or not. A reference type will expect a resource id to be
            // provided, or an inline resource definition.
            if (std::get<1>(type)) {
                // The field type is a reference.
                if (!parser.expect({ expectation(lexeme::res_id).be_true() })) {
                    auto lx = parser.peek();
                    log::fatal_error(lx, 1, "The field '" + field_name.text() + "' expects a resource id.");
                }
                auto id = parser.read();

                // The internal type should be a signed integer type.
                auto value = field.value_at(0);
                auto symbol = type_container.template_field_named(value.name_lexeme());

                switch (std::get<1>(symbol)) {
                    case kdl::DWRD: {
                        resource_data.write_signed_short(id.value<int16_t>());
                        break;
                    }
                    case kdl::DQAD: {
                        resource_data.write_signed_short(id.value<int64_t>());
                        break;
                    }
                    default: {
                        log::fatal_error(field_name, 1, "Resource Reference field should be backed by either a DWRD or DQAD value.");
                    }
                }
            }
            else if (std::get<0>(type) == "File" || std::get<0>(type) == "file") {
                // The field type is the builtin File type. The value should be a string representing a file
                // path. The contents of the file should be loaded into the field as the value.
                auto import_file = false;
                if (parser.expect({ expectation(lexeme::identifier, "import").be_true() })) {
                    parser.advance();
                    import_file = true;
                }

                if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                    auto lx = parser.peek();
                    log::fatal_error(lx, 1, "Fields with the 'File' type expect a string value.");
                }
                auto file_path = parser.read().text();

                // TODO: Import file data and correctly encode it.
            }
            else if (std::get<0>(type) == "Picture" || std::get<0>(type) == "Picture") {
                // The field type is the builtin Picture type. The value should be a string representing an image file
                // path. The contents of the file should be loaded, and then encoded as a PICT resource.
                if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                    auto lx = parser.peek();
                    log::fatal_error(lx, 1, "Fields with the 'Picture' type expect a string value.");
                }
                auto file_path = parser.read();

                // TODO: Import file data and correctly encode it.
                log::fatal_error(file_path, 1, "Picture types are not currently supported.");
            }
            else if (std::get<0>(type) == "Sprite" || std::get<0>(type) == "Sprite") {
                log::fatal_error(parser.peek(), 1, "Sprite types are not currently supported.");
            }
            else if (std::get<0>(type) == "ColorIcon" || std::get<0>(type) == "ColorIcon") {
                // The field type is the builtin ColorIcon type. The value should be a string representing an image file
                // path. The contents of the file should be loaded, and then encoded as a CICN resource.
                if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                    auto lx = parser.peek();
                    log::fatal_error(lx, 1, "Fields with the 'ColorIcon' type expect a string value.");
                }
                auto file_path = parser.read();

                // TODO: Import file data and correctly encode it.
                log::fatal_error(file_path, 1, "ColorIcon types are not currently supported.");
            }
            else {
                log::fatal_error(parser.peek(), 1, "Unknown field type: '" + std::get<0>(type) + "'");
            }
        }
        else {
            // No explicit field value. It needs to be inferred from the expected values.
            for (auto i = 0; i < field.value_count(); ++i) {
                auto value = field.value_at(i);
                auto symbol = type_container.template_field_named(value.name_lexeme());

                switch (std::get<1>(symbol)) {
                    case kdl::DBYT: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_signed_byte(parser.read().value<int8_t>());
                        break;
                    }
                    case kdl::DWRD: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_signed_short(parser.read().value<int16_t>());
                        break;
                    }
                    case kdl::DLNG: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_signed_long(parser.read().value<int32_t>());
                        break;
                    }
                    case kdl::DQAD: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_signed_quad(parser.read().value<int64_t>());
                        break;
                    }
                    case kdl::HBYT: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_byte(parser.read().value<uint8_t>());
                        break;
                    }
                    case kdl::HWRD: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_short(parser.read().value<uint16_t>());
                        break;
                    }
                    case kdl::HLNG: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_long(parser.read().value<uint32_t>());
                        break;
                    }
                    case kdl::HQAD: {
                        if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an integer literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_quad(parser.read().value<uint64_t>());
                        break;
                    }

                    case kdl::PSTR: {
                        if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an string literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_pstr(parser.read().text());
                        break;
                    }
                    case kdl::CSTR: {
                        if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an string literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_cstr(parser.read().text());
                        break;
                    }

                    case kdl::Cxxx: {
                        if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected an string literal for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_cstr(parser.read().text(), type_size(std::get<1>(symbol)));
                        break;
                    }

                    case kdl::RECT: {
                        if (!parser.expect({
                            expectation(lexeme::integer).be_true(),
                            expectation(lexeme::integer).be_true(),
                            expectation(lexeme::integer).be_true(),
                            expectation(lexeme::integer).be_true()
                        })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Expected 4 integer literals for field '" + field_name.text() + "'.");
                        }
                        resource_data.write_signed_short(parser.read().value<int16_t>());
                        resource_data.write_signed_short(parser.read().value<int16_t>());
                        resource_data.write_signed_short(parser.read().value<int16_t>());
                        resource_data.write_signed_short(parser.read().value<int16_t>());
                        break;
                    }

                    case kdl::HEXD: {
                        log::fatal_error(parser.peek(), 1, "The 'HEXD' type is not directly supported '" + field_name.text() + "'.");
                    }

                    case NONE: {
                        log::fatal_error(parser.peek(), 1, "Unknown type encountered in field '" + field_name.text() + "'.");
                    }
                }
            }
        }

        parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    parser.ensure({ expectation(lexeme::r_brace).be_true() });
}
