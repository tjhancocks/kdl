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

#include <stdexcept>
#include "diagnostic/fatal.hpp"
#include "parser/parser.hpp"
#include "parser/sema/declarations/declaration_parser.hpp"
#include "parser/sema/directives/directive_parser.hpp"
#include "parser/sema/type_definition/type_definition_parser.hpp"
#include "parser/sema/component/component_parser.hpp"

// MARK: - Constructor

kdl::sema::parser::parser(std::weak_ptr<kdl::target> target, const std::vector<kdl::lexeme>& lexemes)
    : m_target(target), m_lexemes(lexemes), m_ptr(0)
{

}

// MARK: - Parser Base

auto kdl::sema::parser::parse() -> void
{
    auto target = m_target.lock();
    m_ptr = 0;

    while (!finished()) {

        if (expect({ expectation(lexeme::directive, "type").be_true() })) {
            auto container = type_definition_parser(*this, m_target).parse(true);
            target->add_type_container(container);
        }
        else if (expect({ expectation(lexeme::directive, "example").be_true(), expectation(lexeme::identifier, "declare").be_true() })) {
            advance();
            declaration_parser(*this, m_target, true).parse();
        }
        else if (expect_any({ expectation(lexeme::identifier, "component").be_true(), expectation(lexeme::directive, "lua_export").be_true() })) {
            component_parser(*this, m_target).parse();
        }
        else if (expect({ expectation(lexeme::directive).be_true() })) {
            asm_directive(*this, m_target).parse();
        }
        else if (expect({ expectation(lexeme::identifier, "declare").be_true() })) {
            declaration_parser(*this, m_target).parse();
        }
        else {
            auto lx = peek();
            log::fatal_error(lx, 1, "Unexpected lexeme '" + lx.text() + "' encountered.");
        }

        ensure({ expectation(lexeme::semi).be_true() });
    }
}

// MARK: - Parser Private

auto kdl::sema::parser::finished(long offset, long count) const -> bool
{
    auto ptr = (m_ptr + offset);
    auto end_ptr = (ptr + count);
    auto size = m_lexemes.size();
    return ptr > size || end_ptr > size;
}

auto kdl::sema::parser::consume(kdl::sema::expectation::function expect) -> std::vector<lexeme>
{
    std::vector<lexeme> v;
    while (!finished() && expect(peek())) {
        v.emplace_back(read());
    }
    return v;
}

auto kdl::sema::parser::advance(long delta) -> void
{
    for (auto n = 0; n < delta; ++n) {
        if (!m_tmp_lexemes.empty()) {
            m_tmp_lexemes.erase(m_tmp_lexemes.begin());
        }
        else {
            m_ptr++;
        }
    }
}

auto kdl::sema::parser::push(std::initializer_list<lexeme> lexemes) -> void
{
    m_tmp_lexemes = std::vector<lexeme>(lexemes);
}

auto kdl::sema::parser::clear_pushed_lexemes() -> void
{
    m_tmp_lexemes.clear();
}

auto kdl::sema::parser::peek(long offset) const -> kdl::lexeme
{
    if (!m_tmp_lexemes.empty() && (offset >= 0 || offset < m_tmp_lexemes.size())) {
        return m_tmp_lexemes.at(offset);
    }
    if (finished(offset, 1)) {
        throw std::logic_error("[kdl::sema::parser] Attempted to access lexeme beyond end of stream.");
    }
    return m_lexemes[m_ptr + offset];
}

auto kdl::sema::parser::read(long offset) -> kdl::lexeme
{
    auto Tk = peek(offset);
    if (m_tmp_lexemes.empty() || (offset >= m_tmp_lexemes.size())) {
        advance(offset + 1);
    }
    else {
        for (auto n = 0; n <= offset; ++n) {
            m_tmp_lexemes.erase(m_tmp_lexemes.begin());
        }
    }
    return Tk;
}

auto kdl::sema::parser::expect(std::initializer_list<kdl::sema::expectation::function> expect) const -> bool
{
    auto ptr = 0;
    for (auto f : expect) {
        if (f(peek(ptr++)) == false) {
            return false;
        }
    }
    return true;
}

auto kdl::sema::parser::expect_any(std::initializer_list<kdl::sema::expectation::function> expect) const -> bool
{
    return expect_any(std::vector(expect));
}

auto kdl::sema::parser::expect_any(std::vector<expectation::function> expect) const -> bool
{
    for (auto f : expect) {
        if (f(peek())) {
            return true;
        }
    }
    return false;
}

auto kdl::sema::parser::ensure(std::initializer_list<kdl::sema::expectation::function> expect) -> void
{
    for (auto f : expect) {
        auto Tk = read();
        if (f(Tk) == false) {
            log::fatal_error(Tk, 1, "Could not ensure the correctness of the token '" + Tk.text() + "'");
        }
    }
}

// MARK: - Lexeme Insertion

auto kdl::sema::parser::insert(std::vector<lexeme> lexemes, const int offset) -> void
{
    if (finished(offset, 1)) {
        // We trying to insert at the end of the stream.
        m_lexemes.insert(m_lexemes.end(), lexemes.begin(), lexemes.end());
    }
    else {
        // We're inserting mid stream.
        auto insertion_ptr = m_lexemes.begin() + m_ptr + offset;
        m_lexemes.insert(insertion_ptr, lexemes.begin(), lexemes.end());
    }
}

// MARK: - Accessors

auto kdl::sema::parser::size() const -> std::size_t
{
    return m_lexemes.size();
}