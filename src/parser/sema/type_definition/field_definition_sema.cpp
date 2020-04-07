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

#include "diagnostic/fatal.hpp"
#include "field_definition_sema.hpp"

auto kdl::sema::field_definition_sema::parse(kdl::sema::parser &parser, kdl::container &type_container) -> void
{
    // We can safely assume that the 'field' keyword has been consumed.

    // The first thing that needs to be parsed is the field name. This is structured like so:
    //  '(' STRING ')'
    parser.ensure({ expectation(lexeme::l_paren).be_true() });

    if (!parser.expect({ expectation(lexeme::string).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Field name must be a string.");
    }
    auto field_name = parser.read();
    kdl::field field(field_name);

    parser.ensure({ expectation(lexeme::r_paren).be_true() });

    // The field body is wrapped in '{' braces '}'. This body is required.
    parser.ensure({ expectation(lexeme::l_brace).be_true() });

    while (parser.expect({ expectation(lexeme::r_brace).be_false() })) {

        if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
            auto lx = parser.peek();
            log::fatal_error(lx, 1, "Expected an indentifier in field body.");
        }
        auto value_label = parser.read();
        field_value ref(value_label, {}, {});

        // Is there an explicit type associated with this field_value?
        if (parser.expect({ expectation(lexeme::identifier, "as").be_true() })) {
            parser.advance();
            field_value_type value_type(false);

            if (parser.expect({ expectation(lexeme::identifier).be_true() })) {
                value_type.set_name(parser.read());

                // Do we have associated values/types with the named type?
                if (parser.expect({ expectation(lexeme::l_angle).be_true() })) {
                    parser.advance();

                    while (parser.expect({ expectation(lexeme::r_angle).be_false() })) {
                        if (!parser.expect_any({
                            expectation(lexeme::identifier).be_true(),
                            expectation(lexeme::integer).be_true(),
                            expectation(lexeme::percentage).be_true(),
                            expectation(lexeme::res_id).be_true()
                        })) {
                            auto lx = parser.peek();
                            log::fatal_error(lx, 1, "Associated values on a type must be either an Identifier or Numeric value.");
                        }
                        auto assoc = parser.read();
                        value_type.add_associated(assoc);

                        if (parser.expect({ expectation(lexeme::r_angle).be_false() })) {
                            if (parser.expect_any({
                                expectation(lexeme::comma).be_true(),
                                expectation(lexeme::pipe).be_true()
                            })) {
                                parser.advance();
                            }
                            else {
                                log::fatal_error(parser.peek(), 1, "Multiple associated values on a type must be deliminated by either | or ,");
                            }
                        }
                    }

                    parser.ensure({ expectation(lexeme::r_angle).be_true() });
                }
            }

            if (parser.expect({ expectation(lexeme::amp).be_true() })) {
                parser.advance();
                value_type.set_is_reference(true);
            }

            ref.set_type(value_type);
        }

        // Is there a default value associated with this value.
        if (parser.expect({ expectation(lexeme::equals).be_true() })) {
            parser.advance();

            // TODO: check that the lexeme is the correct type for the value, or an identifier if a symbol list is present.

            auto default_value = parser.read();
            ref.set_default_value(default_value);
        }

        // Check if a symbol list is present.
        if (parser.expect({ expectation(lexeme::l_bracket).be_true() })) {
            parser.advance();

            while (parser.expect({ expectation(lexeme::r_bracket).be_false() })) {

                if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
                    auto lx = parser.peek();
                    log::fatal_error(lx, 1, "Symbol name must be an identifier.");
                }
                auto symbol = parser.read();

                parser.ensure({ expectation(lexeme::equals).be_true() });

                if (parser.expect({ expectation(lexeme::integer).be_true() })
                    || parser.expect({ expectation(lexeme::string).be_true() })
                    || parser.expect({ expectation(lexeme::res_id).be_true() })
                    || parser.expect({ expectation(lexeme::percentage).be_true() })
                ) {
                    ref.add_symbol(symbol, parser.read());
                }
                else {
                    auto lx = parser.peek();
                    log::fatal_error(lx, 1, "Symbol value must be a string, integer, resource id or percentage.");
                }

                if (parser.expect({ expectation(lexeme::r_bracket).be_false() })) {
                    parser.ensure({ expectation(lexeme::comma).be_true() });
                }
            }

            parser.ensure({ expectation(lexeme::r_bracket).be_true() });
        }

        // Record the value in the field.
        field.add_value(ref);

        parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    parser.ensure({ expectation(lexeme::r_brace).be_true() });

    type_container.add_field(field);
}
