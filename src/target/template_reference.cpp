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
#include "target/template_reference.hpp"

// MARK: - Constructor

kdl::template_reference::template_reference(const kdl::lexeme name, std::optional<lexeme> default_value)
    : m_name(name), m_default(default_value)
{

}

// MARK: - Accessors

auto kdl::template_reference::name() const -> std::string
{
    return m_name.text();
}

auto kdl::template_reference::name_lexeme() const -> lexeme
{
    return m_name;
}

auto kdl::template_reference::default_value() const -> std::optional<lexeme>
{
    return m_default;
}

auto kdl::template_reference::set_default_value(std::optional<lexeme> default_value) -> void
{
    m_default = default_value;
}

// MARK: - Symbol Management

auto kdl::template_reference::add_symbol(const kdl::lexeme symbol, const kdl::lexeme value) -> void
{
    for (auto s : m_symbols) {
        if (std::get<0>(s).text() == symbol.text()) {
            log::fatal_error(symbol, 1, "Duplicated symbol name '" + symbol.text() + "'");
        }
    }
    m_symbols.emplace_back(std::make_tuple(symbol, value));
}

auto kdl::template_reference::value_for(const kdl::lexeme symbol) const -> kdl::lexeme
{
    for (auto s : m_symbols) {
        if (std::get<0>(s).text() == symbol.text()) {
            return std::get<1>(s);
        }
    }
    log::fatal_error(symbol, 1, "Unrecognised symbol '" + symbol.text() + "'");
}

auto kdl::template_reference::symbol_count() const -> std::size_t
{
    return m_symbols.size();
}

auto kdl::template_reference::symbol_at(const int i) -> std::tuple<lexeme, lexeme>
{
    return m_symbols[i];
}

