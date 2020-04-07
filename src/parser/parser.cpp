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
#include "parser/parser.hpp"
#include "parser/sema/directives/asm_directive_sema.hpp"
#include "parser/sema/declaration/declaration_sema.hpp"

// MARK: - Constructor

kdl::sema::parser::parser(std::weak_ptr<kdl::target> target, std::vector<kdl::lexeme> lexemes)
    : m_target(target), m_lexemes(lexemes), m_ptr(0)
{

}

// MARK: - Parser Base

auto kdl::sema::parser::parse() -> void
{
    m_ptr = 0;

    while (!finished()) {

        if (asm_directive_sema::test(*this)) {
            asm_directive_sema::parse(*this, m_target);
        }
        else if (declaration_sema::test(*this)) {
            declaration_sema::parse(*this, m_target);
        }
        else {
            auto lx = peek();
            log::fatal_error(lx, 1, "Unexpected lexeme '" + lx.text() + "' encountered.");
        }

    }
}

// MARK: - Parser Private

auto kdl::sema::parser::finished(const long offset, const long count) const -> bool
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

auto kdl::sema::parser::advance(const long delta) -> void
{
    m_ptr += delta;
    m_tmp_lexeme = {};
}

auto kdl::sema::parser::push(const lexeme lexeme) -> void
{
    m_tmp_lexeme = lexeme;
}

auto kdl::sema::parser::peek(const long offset) const -> kdl::lexeme
{
    if (m_tmp_lexeme.has_value() && offset == 0) {
        return m_tmp_lexeme.value();
    }
    if (finished(offset, 1)) {
        throw std::logic_error("[kdl::sema::parser] Attempted to access lexeme beyond end of stream.");
    }
    return m_lexemes[m_ptr + offset];
}

auto kdl::sema::parser::read(const long offset) -> kdl::lexeme
{
    auto Tk = peek(offset);
    if (!m_tmp_lexeme.has_value()) {
        advance(offset + 1);
    }
    else {
        m_tmp_lexeme = {};
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
