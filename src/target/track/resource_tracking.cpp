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

#include <iostream>
#include <algorithm>
#include "target/track/resource_tracking.hpp"

// MARK: - Instance Construction

kdl::resource_tracking::table::instance::instance(std::string file, std::string type, int64_t id, std::string name)
    : file(std::move(file)), type_code(std::move(type)), id(id), name(std::move(name))
{

}


// MARK: - Instance Management

auto kdl::resource_tracking::table::add_instance(const std::string &file,
                                                 const std::string &type,
                                                 const int64_t &id,
                                                 const std::string &name) -> void
{
    m_instances.emplace_back(instance(file, type, id, name));
}

auto kdl::resource_tracking::table::instance_exists(const std::string &type, const int64_t &id) -> bool
{
    return std::any_of(m_instances.begin(), m_instances.end(), [type, id] (const instance& i) {
        return (i.type_code == type) && (i.id == id);
    });
}
