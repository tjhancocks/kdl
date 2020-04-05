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
#include "parser/lexer.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Constructor

kdl::lexer::lexer(std::shared_ptr<file> source)
    : m_source(source)
{

}

// MARK: - Lexical Analysis

auto kdl::lexer::analyze() -> std::vector<lexeme>
{
    if (!m_lexemes.empty()) {
        return m_lexemes;
    }

    // Loop through the source code as long as there are characters available to consume.
    while (available()) {

        // Consume any leading whitespace
        consume_while(set<' ', '\t'>::contains);

        // Check if we're looking at a newline. If we are the simply consume it and increment the current line number.
        if (test_if(match<'\n'>::yes)) {
            advance();
            m_line++;
            m_offset = 0;
            continue;
        }

        // Check for a comment. If we're looking at a comment then we need to consume the entire line. We need to
        // advance past the character at the end of the match.
        if (test_if(match<'`'>::yes)) {
            consume_while(match<'\n'>::no);
            continue;
        }

        // Constructs
        else if (test_if(match<'@'>::yes)) {
            // We're looking at a directive.
            // Directive's are formed of a @ followed an identifier.
            advance();
            consume_while(identifier_set::contains);
            m_lexemes.emplace_back(kdl::lexeme(m_slice, lexeme::directive, m_pos, m_offset, m_line, m_source));
        }

        // Literals
        else if (test_if(match<'"'>::yes)) {
            // We're looking at a string literal.
            // The string continues until a corresponding '"' is found.
            advance();
            consume_while(match<'"'>::no);
            m_lexemes.emplace_back(kdl::lexeme(m_slice, lexeme::string, m_pos, m_offset, m_line, m_source));
            advance();
        }
        else if (test_if(match<'#'>::yes)) {
            // We're looking at a resource id.
            advance();
            consume_while(decimal_set::contains);
            m_lexemes.emplace_back(kdl::lexeme(m_slice, lexeme::res_id, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'$'>::yes)) {
            // We're looking at a variable.
            advance();
            consume_while(identifier_set::contains);
            m_lexemes.emplace_back(kdl::lexeme(m_slice, lexeme::var, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'0'>::yes) && test_if(set<'x', 'X'>::contains, 1)) {
            // We're looking at a hexadecimal number
            advance(2);
            consume_while(hexadecimal_set::contains);
            m_lexemes.emplace_back(kdl::lexeme("0x" + m_slice, lexeme::integer, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(decimal_set::contains) || (test_if(match<'-'>::yes) && test_if(decimal_set::contains, 1))) {
            // We're looking at a number
            auto negative = test_if(match<'-'>::yes);
            if (negative) {
                advance();
            }

            consume_while(decimal_set::contains);
            auto number_text = m_slice;

            if (test_if(match<'%'>::yes)) {
                // This is a percentage.
                advance();
                m_lexemes.emplace_back(kdl::lexeme(m_slice, lexeme::percentage, m_pos, m_offset, m_line, m_source));
            }
            else {
                m_lexemes.emplace_back(kdl::lexeme(m_slice, lexeme::integer, m_pos, m_offset, m_line, m_source));
            }
        }
        else if (test_if(identifier_set::contains)) {
            consume_while(identifier_set::contains);

            // TODO: Check for keywords

            m_lexemes.emplace_back(kdl::lexeme(m_slice, lexeme::identifier, m_pos, m_offset, m_line, m_source));
        }

        // Symbols
        else if (test_if(match<';'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::semi, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'{'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::l_brace, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'}'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::r_brace, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'['>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::l_bracket, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<']'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::r_bracket, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'('>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::l_paren, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<')'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::r_paren, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'<'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::l_angle, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'>'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::r_angle, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'='>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::equals, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'+'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::plus, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'-'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::minus, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'*'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::star, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'/'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::slash, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'&'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::amp, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'.'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::dot, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<','>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::comma, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'|'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::pipe, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<'^'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::carat, m_pos, m_offset, m_line, m_source));
        }
        else if (test_if(match<':'>::yes)) {
            m_lexemes.emplace_back(kdl::lexeme(read(), lexeme::colon, m_pos, m_offset, m_line, m_source));
        }

        // Unrecognised character encountered
        else {
            log::fatal_error(dummy(), 1, "Unrecognised character '" + peek() + "' encountered.");
        }
    }

    return m_lexemes;
}

// MARK: - Private Lexer

auto kdl::lexer::dummy(const long offset) const -> kdl::lexeme
{
    return lexeme("(dummy)", lexeme::star, m_pos + offset, m_offset + offset, m_line, m_source);
}

auto kdl::lexer::length() const -> std::size_t
{
    return m_source->contents().size();
}

auto kdl::lexer::advance(const long offset) -> void
{
    m_pos += offset;
    m_offset += offset;
}

auto kdl::lexer::available(const long offset, std::size_t length) const -> bool
{
    auto start = m_pos + offset;
    auto end = start + length;
    return (end <= this->length());
}

auto kdl::lexer::peek(const long offset, std::size_t length) const -> std::string
{
    if (!available(offset, length)) {
        kdl::log::fatal_error(dummy(offset), 1, "Failed to peek '" + std::to_string(length) + "' characters from source.");
    }
    return m_source->contents().substr(m_pos + offset, length);
}

auto kdl::lexer::read(const long offset, std::size_t length) -> std::string
{
    auto str = peek(offset, length);
    advance(offset + length);
    return str;
}

auto kdl::lexer::test_if(std::function<auto(const std::string) -> bool> fn, const long offset, std::size_t length) const -> bool
{
    return fn(peek(offset, length));
}

auto kdl::lexer::consume_while(std::function<auto(const std::string) -> bool> fn) -> bool
{
    m_slice.clear();
    while (fn(peek())) {
        m_slice += read();
    }
    return !m_slice.empty();
}

// MARK: -

template<char c>
auto kdl::match<c>::yes(const std::string __Chk) -> bool
{
    return __Chk == std::string(1, c);
}


template <char c>
auto kdl::match<c>::no(const std::string __Chk) -> bool
{
    return !yes(__Chk);
}

template <char lc, char uc>
auto kdl::range<lc, uc>::contains(const std::string __Chk) -> bool
{
    for (auto __ch : __Chk) {
        if (__ch < lc || __ch > uc) {
            return false;
        }
    }
    return true;
}

template <char lc, char uc>
auto kdl::range<lc, uc>::not_contains(const std::string __Chk) -> bool
{
    return !contains(__Chk);
}

template<char tC, char... ttC>
auto kdl::set<tC, ttC...>::contains(const std::string __Chk) -> bool
{
    std::vector<char> v = {tC, ttC...};

    for (auto __ch : __Chk) {
        if (std::find(v.begin(), v.end(), __ch) == v.end()) {
            return false;
        }
    }

    return true;
}

template<char tC, char... ttC>
auto kdl::set<tC, ttC...>::not_contains(const std::string __Chk) -> bool
{
    return !contains(__Chk);
}

auto kdl::identifier_set::contains(const std::string __Chk) -> bool
{
    for (auto __ch : __Chk) {
        auto condition = (__ch >= 'A' && __ch <= 'Z') || (__ch >= 'a' && __ch <= 'z') || __ch == '_';
        if (!condition) {
            return false;
        }
    }
    return true;
}

auto kdl::decimal_set::contains(const std::string __Chk) -> bool
{
    for (auto __ch : __Chk) {
        auto condition = (__ch >= '0' && __ch <= '9');
        if (!condition) {
            return false;
        }
    }
    return true;
}

auto kdl::hexadecimal_set::contains(const std::string __Chk) -> bool
{
    for (auto __ch : __Chk) {
        auto condition = (__ch >= 'A' && __ch <= 'F') || (__ch >= 'a' && __ch <= 'f') || (__ch >= '0' && __ch <= '9');
        if (!condition) {
            return false;
        }
    }
    return true;
}