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

#include "target/target.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Constructors

kdl::target::target()
{

}

kdl::target::target(const std::string path)
    : m_path(path)
{

}

// MARK: - Container Management

auto kdl::target::add_container(const kdl::container container) -> void
{
    m_containers.emplace_back(container);
}

auto kdl::target::container_count() const -> std::size_t
{
    return m_containers.size();
}

auto kdl::target::container_at(const int i) const -> kdl::container
{
    return m_containers[i];
}

auto kdl::target::container_named(const kdl::lexeme name) const -> kdl::container
{
    for (auto c : m_containers) {
        if (c.name() == name.text()) {
            return c;
        }
    }
    log::fatal_error(name, 1, "Missing definition for type '" + name.text() + "'");
}

