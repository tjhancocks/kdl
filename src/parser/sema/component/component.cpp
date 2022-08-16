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

#include <iostream>
#include <utility>
#include "parser/sema/component/component.hpp"
#include "codegen/lua/type_exporter.hpp"

// MARK: - Construction

kdl::sema::component::component(std::string name, enum mode mode)
    : m_name(std::move(name)), m_mode(mode)
{

}

// MARK: - Accessors

auto kdl::sema::component::name() const -> std::string
{
    return m_name;
}

auto kdl::sema::component::set_path_prefix(const std::string &path) -> void
{
    m_path_prefix = path;
}

auto kdl::sema::component::path_prefix() const -> std::string
{
    return m_path_prefix;
}

auto kdl::sema::component::set_namespace(const std::string& ns) -> void
{
    m_namespace = ns;
}

auto kdl::sema::component::ns() const -> std::string
{
    return m_namespace;
}

auto kdl::sema::component::set_base_id(const int64_t id) -> void
{
    m_base_id = id;
}

auto kdl::sema::component::base_id() const -> int64_t
{
    return m_base_id;
}

auto kdl::sema::component::set_type(const kdl::lexeme& type) -> void
{
    m_as_type = type;
}

auto kdl::sema::component::type() const -> kdl::lexeme
{
    return m_as_type;
}

auto kdl::sema::component::add_file(const std::string& path) -> void
{
    m_files.emplace_back(path);
}

auto kdl::sema::component::add_file(const std::string& path, const std::string& name) -> void
{
    m_files.emplace_back(path, name);
}

auto kdl::sema::component::files() const -> std::vector<file>
{
    return m_files;
}

auto kdl::sema::component::set_files(std::vector<file> files) -> void
{
    m_files = std::move(files);
}

// MARK: - Types

auto kdl::sema::component::set_export_types(const std::vector<lexeme> &types) -> void
{
    m_export_types = types;
}

// MARK: - Resource Generation

auto kdl::sema::component::generate_resources(const std::shared_ptr<target>& target) const -> void
{
    // Fetch the type container for the resources...
    auto container = target->type_container_named(m_as_type);

    // Iterate through each of the files and produce a resource for it.
    int64_t id = m_base_id;
    for (const auto& file : m_files) {
        const auto& path = target->resolve_src_path(m_path_prefix + file.path);
        const auto& contents = kdl::file(path).contents();

        build_target::resource_constructor resource(id++,
                                                    container.code(),
                                                    file.name.has_value() ? file.name.value() : "",
                                                    contents);

        // Set up the attributes of the resource.
        resource.set_attribute("namespace", m_namespace);

        target->add_resource(resource);
    }
}

// MARK: - Lua Generation

auto kdl::sema::component::synthesize_lua_from_types(const std::shared_ptr<target> &target) const -> void
{
    // Fetch the type container for the resources...
    auto container = target->type_container_named(m_as_type);

    // Iterate through each of the types and produce a resource for it.
    int64_t id = m_base_id;
    for (const auto& type_name : m_export_types) {
        auto type = target->type_container_named(type_name);
        codegen::lua::type_exporter exporter(type);
        auto lua = exporter.generate_lua();

        build_target::resource_constructor resource(id++,
                                                    container.code(),
                                                    type_name.text(),
                                                    lua);

        // Set up the attributes of the resource.
        resource.set_attribute("namespace", m_namespace);

        target->add_resource(resource);
    }
}