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

#include <algorithm>
#include <libKDL/lexer/lexer.hpp>
#include <libKDL/lexer/conditions/match.hpp>
#include <libKDL/lexer/conditions/sequence.hpp>
#include <libKDL/lexer/conditions/range.hpp>
#include <libKDL/lexer/conditions/set.hpp>
#include <libKDL/lexer/conditions/identifier_set.hpp>
#include <libKDL/lexer/conditions/decimal_set.hpp>
#include <libKDL/lexer/conditions/hexadecimal_set.hpp>
#include <libKDL/exception/lexical_exception.hpp>
#include <libKDL/exception/unrecognised_character_exception.hpp>

// MARK: - Constructor

kdl::lexer::lexer::lexer(const std::shared_ptr<host::filesystem::file>& source)
    : m_source(source)
{
}

// MARK: - Lexical Analysis

auto kdl::lexer::lexer::analyze() -> std::vector<lexeme>
{
    if (!m_lexemes.empty()) {
        return m_lexemes;
    }

    // Loop through the source code as long as there are characters available to consume.
    while (available()) {

        // Consume any leading whitespace
        consume_while(condition::set<' ', '\t'>::contains);

        // Check if we're looking at a newline. If we are the simply consume it and increment the current line number.
        if (test_if(condition::match<'\n'>::yes)) {
            advance();
            m_line++;
            m_offset = 0;
            continue;
        }
        else if (test_if(condition::match<'\r'>::yes)) {
            advance();
            continue;
        }

        // Check for a comment. If we're looking at a comment then we need to consume the entire line. We need to
        // advance past the character at the end of the match.
        if (test_if(condition::match<'`'>::yes)) {
            consume_while(condition::match<'\n'>::no);
            continue;
        }

        // Constructs
        else if (test_if(condition::match<'@'>::yes)) {
            // We're looking at a directive.
            // Directive's are formed of a @ followed an identifier.
            advance();
            consume_while(condition::identifier_set::contains);
            m_lexemes.emplace_back(lexeme(m_slice, lexeme::directive, m_pos, m_offset, m_line, m_source));
        }

        // Literals
        else if (test_if(condition::match<'"'>::yes)) {
            // We're looking at a string literal.
            // The string continues until a corresponding '"' is found.
            advance();
            consume_while(condition::match<'"'>::no);
            m_lexemes.emplace_back(lexeme(m_slice, lexeme::string, m_pos, m_offset, m_line, m_source));
            advance();
        }
        else if (available(0, 5) && test_if([](const std::string hint) -> bool{ return hint == "#auto"; }, 0, 5)) {
            // TODO: Expand upon the lexer to handle keywords of this format.
            m_lexemes.emplace_back(lexeme(read(1, 4), lexeme::res_id, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'#'>::yes)) {
            // We're looking at a resource id.
            // Check if there is a namespace and a type present. The general format for a resource id is as follows:
            //  #[Namespace.][Type.]ID
            // Each prefix entry will be determined to be either a namespace or a type based on the existence of an
            // existing type with the same name.
            advance();

            std::vector<std::string> components;
            if (test_if(condition::identifier_set::limited_contains)) {
                consume_while(condition::identifier_set::contains);
                components.emplace_back(m_slice);
                advance();
            }

            if (test_if(condition::identifier_set::limited_contains)) {
                consume_while(condition::identifier_set::contains);
                components.emplace_back(m_slice);
                advance();
            }

            auto negative = test_if(condition::match<'-'>::yes);
            if (negative) {
                advance();
            }

            consume_while(condition::decimal_set::contains);
            if (negative) {
                components.emplace_back("-" + m_slice);
            }
            else {
                components.emplace_back(m_slice);
            }

            m_lexemes.emplace_back(lexeme(components, lexeme::res_id, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'$'>::yes) && !m_in_expr) {
            // We're looking at a variable or an expression.
            advance();

            if (test_if(condition::match<'('>::yes)) {
                advance();
                // We're looking at an explicit expression.
                m_lexemes.emplace_back(lexeme(m_slice, lexeme::l_expr, m_pos, m_offset, m_line, m_source));

                // Set a flag to indicate that we're in an expression.
                m_in_expr = true;
            }
            else {
                // We're looking at an implicit variable expression.
                consume_while(condition::identifier_set::contains);
                m_lexemes.emplace_back(lexeme(m_slice, lexeme::var, m_pos, m_offset, m_line, m_source));
            }
        }
        else if (test_if(condition::match<'0'>::yes) && test_if(condition::set<'x', 'X'>::contains, 1)) {
            // We're looking at a hexadecimal number
            advance(2);
            consume_while(condition::hexadecimal_set::contains);
            m_lexemes.emplace_back(lexeme("0x" + m_slice, lexeme::integer, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::decimal_set::contains) || (test_if(condition::match<'-'>::yes) && test_if(condition::decimal_set::contains, 1))) {
            // We're looking at a number
            auto negative = test_if(condition::match<'-'>::yes);
            if (negative) {
                advance();
            }

            consume_while(condition::decimal_set::contains);
            auto number_text = m_slice;
            if (negative) {
                number_text.insert(0, 1, '-');
            }

            if (test_if(condition::match<'%'>::yes)) {
                // This is a percentage.
                advance();
                m_lexemes.emplace_back(lexeme(number_text, lexeme::percentage, m_pos, m_offset, m_line, m_source));
            }
            else {
                m_lexemes.emplace_back(lexeme(number_text, lexeme::integer, m_pos, m_offset, m_line, m_source));
            }
        }
        else if (test_if(condition::identifier_set::limited_contains)) {
            consume_while(condition::identifier_set::contains);
            m_lexemes.emplace_back(lexeme(m_slice, lexeme::identifier, m_pos, m_offset, m_line, m_source));
        }

        // Symbols
        else if (test_if(condition::match<';'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::semi, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'{'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::l_brace, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'}'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::r_brace, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'['>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::l_bracket, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<']'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::r_bracket, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'('>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::l_paren, m_pos, m_offset, m_line, m_source));
            if (m_in_expr) {
                m_expr_paren_balance++;
            }
        }
        else if (test_if(condition::match<')'>::yes) && m_in_expr && m_expr_paren_balance <= 0) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::r_expr, m_pos, m_offset, m_line, m_source));
            m_in_expr = false;
        }
        else if (test_if(condition::match<')'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::r_paren, m_pos, m_offset, m_line, m_source));
            if (m_in_expr) {
                m_expr_paren_balance--;
            }
        }
        else if (test_if(condition::match<'<'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::l_angle, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'>'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::r_angle, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'='>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::equals, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'+'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::plus, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'-'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::minus, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'*'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::star, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'/'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::slash, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'&'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::amp, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'.'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::dot, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<','>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::comma, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'|'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::pipe, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'^'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::carat, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<':'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::colon, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(condition::match<'!'>::yes)) {
            m_lexemes.emplace_back(lexeme(read(), lexeme::exclaim, m_pos, m_offset, m_line, m_source));
        }

        // Unrecognised character encountered
        else {
            throw unrecognised_character_exception(dummy());
        }
    }

    return m_lexemes;
}

// MARK: - Private Lexer

auto kdl::lexer::lexer::dummy(long offset) const -> lexeme
{
    return { peek(), lexeme::any, m_pos + offset, m_offset + offset, m_line, m_source };
}

auto kdl::lexer::lexer::length() const -> std::size_t
{
    return m_source->string_contents().size();
}

auto kdl::lexer::lexer::advance(long offset) -> void
{
    m_pos += offset;
    m_offset += offset;
}

auto kdl::lexer::lexer::available(long offset, std::size_t length) const -> bool
{
    auto start = m_pos + offset;
    auto end = start + length;
    return (end <= this->length());
}

auto kdl::lexer::lexer::peek(long offset, std::size_t length) const -> std::string
{
    if (!available(offset, length)) {
        throw lexical_exception("Failed to peek " + std::to_string(length) + " characters from source.", dummy(offset));
    }
    return m_source->string_contents().substr(m_pos + offset, length);
}

auto kdl::lexer::lexer::read(long offset, std::size_t length) -> std::string
{
    auto str = peek(offset, length);
    advance(offset + length);
    return str;
}

auto kdl::lexer::lexer::test_if(const std::function<auto(const std::string&) -> bool>& fn, long offset, std::size_t length) const -> bool
{
    return fn(peek(offset, length));
}

auto kdl::lexer::lexer::consume_while(const std::function<auto(const std::string&) -> bool>& fn, std::size_t size) -> bool
{
    m_slice.clear();
    while (fn(peek(0, size))) {
        m_slice += read(0, size);
    }
    return !m_slice.empty();
}
