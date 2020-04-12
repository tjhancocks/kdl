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

// MARK: - Constructor

kdl::sema::list_parser::list_parser(parser& parser)
    : m_parser(parser)
{

}

// MARK: - Configuration

auto kdl::sema::list_parser::set_list_start(enum lexeme::type lx) -> void
{
    m_list_start = lx;
}

auto kdl::sema::list_parser::set_list_end(enum lexeme::type lx) -> void
{
    m_list_end = lx;
}

auto kdl::sema::list_parser::set_delimiter(enum lexeme::type lx) -> void
{
    m_delimit = lx;
}

auto kdl::sema::list_parser::add_valid_list_item(enum lexeme::type lx) -> void
{
    m_valid_lexemes.emplace_back(std::make_tuple(lx, std::make_optional<std::string>()));
}

auto kdl::sema::list_parser::add_valid_list_item(enum lexeme::type lx, const std::string text) -> void
{
    m_valid_lexemes.emplace_back(std::make_tuple(lx, text));
}

// MARK: - Parser

auto kdl::sema::list_parser::parse() -> std::vector<lexeme>
{
    // Build a list of valid expactations
    std::vector<expectation::function> expectations;
    for (auto ex : m_valid_lexemes) {
        auto type = std::get<0>(ex);
        auto text = std::get<1>(ex);

        if (text.has_value()) {
            expectations.emplace_back(expectation(type, text.value()).be_true());
        }
        else {
            expectations.emplace_back(expectation(type).be_true());
        }
    }

    // Parse the list
    std::vector<lexeme> out;
    m_parser.ensure({ expectation(m_list_start).be_true() });
    while (m_parser.expect({ expectation(m_list_end).be_false() })) {
        if (!m_parser.expect_any(expectations)) {
            log::fatal_error(m_parser.peek(), 1, "Unexpected type encountered in list.");
        }
        out.emplace_back(m_parser.read());

        if (m_parser.expect({ expectation(m_list_end).be_false() })) {
            m_parser.ensure({ expectation(m_delimit).be_true() });
        }
    }
    m_parser.ensure({ expectation(m_list_end).be_true() });
    return out;
}