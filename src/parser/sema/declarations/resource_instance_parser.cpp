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

#include "diagnostic/fatal.hpp"
#include "parser/sema/util/list_parser.hpp"
#include "parser/sema/declarations/resource_instance_parser.hpp"
#include "parser/sema/declarations/field_parser.hpp"

// MARK: - Constructor

kdl::sema::resource_instance_parser::resource_instance_parser(kdl::sema::parser &parser,
                                                              kdl::build_target::type_container &type,
                                                              std::weak_ptr<target> target)
    : m_type(type), m_parser(parser), m_target(target)
{

}

// MARK: - Configuration

auto kdl::sema::resource_instance_parser::set_keyword(const std::string keyword) -> void
{
    m_keyword = keyword;
}

auto kdl::sema::resource_instance_parser::set_id(const int64_t id) -> void
{
    m_id = id;
}

auto kdl::sema::resource_instance_parser::set_name(const std::string name) -> void
{
    m_name = name;
}

// MARK: - Parser

auto kdl::sema::resource_instance_parser::parse() -> kdl::build_target::resource_instance
{
    auto first_lx = m_parser.peek();
    m_parser.ensure({
        expectation(lexeme::identifier, m_keyword).be_true()
    });

    if (m_parser.expect({ expectation(lexeme::l_paren).be_true() })) {
        list_parser list(m_parser);
        list.set_list_start(lexeme::l_paren);
        list.set_list_end(lexeme::r_paren);
        list.set_delimiter(lexeme::comma);
        list.add_valid_list_item(lexeme::res_id);
        list.add_valid_list_item(lexeme::string);
        auto args = list.parse();

        for (auto arg : args) {
            if (arg.is(lexeme::res_id)) {
                m_id = arg.value<int64_t>();
            }
            else if (arg.is(lexeme::string)) {
                m_name = arg.text();
            }
        }
    }

    // Acquire a new instance of the resource and populate it with default values.
    auto instance = m_type.new_instance(m_id, m_name);
    for (auto field : m_type.all_fields()) {
        field_parser(m_parser, m_type, instance, m_target).apply_defaults_for_field(field.name());
    }
    m_parser.clear_pushed_lexemes();
    instance.reset_acquistion_locks();

    m_parser.ensure({ expectation(lexeme::l_brace).be_true() });
    while (m_parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        field_parser(m_parser, m_type, instance, m_target).parse();
        m_parser.ensure({ expectation(lexeme::semi).be_true() });
    }
    m_parser.ensure({ expectation(lexeme::r_brace).be_true() });

    // Before we can hand the instance back to the caller, we need to run any assertions on it to ensure it's
    // validity.
    for (auto assertion : m_type.assertions()) {
        if (!assertion.evaluate(instance.synthesize_variables())) {
            log::fatal_error(first_lx, 1, "Assertion Failed: " + assertion.failure_text());
        }
    }

    return instance;
}