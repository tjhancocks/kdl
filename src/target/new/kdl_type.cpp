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

#include "target/new/kdl_type.hpp"

// MARK: - Constructors

kdl::build_target::kdl_type::kdl_type(const bool& is_reference, const kdl::lexeme& name)
    : m_reference(is_reference), m_type_name(name)
{

}

// MARK: - Reference Type : '&'

auto kdl::build_target::kdl_type::set_reference(const bool& reference) -> void
{
    m_reference = reference;
}

auto kdl::build_target::kdl_type::is_reference() const -> bool
{
    return m_reference;
}

// MARK: - Name

auto kdl::build_target::kdl_type::set_name(const kdl::lexeme& name) -> void
{
    m_type_name = name;
}

auto kdl::build_target::kdl_type::name() const -> std::optional<kdl::lexeme>
{
    return m_type_name;
}

// MARK: - Type Hint Management

auto kdl::build_target::kdl_type::set_type_hints(const std::vector<lexeme>& hints) -> void
{
    m_type_hints = hints;
}

auto kdl::build_target::kdl_type::type_hints() const -> std::vector<lexeme>
{
    return m_type_hints;
}






