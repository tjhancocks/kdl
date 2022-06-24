// Copyright (c) 2021 Tom Hancocks
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

#include "parser/sema/component/component_file.hpp"

// MARK: - Construction

kdl::sema::component_file::component_file(const std::string& file)
    : m_file(file), m_name({})
{
    // TODO: Name should probably be taken from the file name and converted into something friendly for a name.
}

kdl::sema::component_file::component_file(const std::string &file, const std::string &name)
    : m_file(file), m_name(name)
{

}

// MARK: - Accessors

auto kdl::sema::component_file::name() const -> std::string
{
    return m_name.has_value() ? m_name.value() : "";
}

auto kdl::sema::component_file::file() const -> std::string
{
    return m_file;
}

auto kdl::sema::component_file::has_name() const -> bool
{
    return m_name.has_value();
}