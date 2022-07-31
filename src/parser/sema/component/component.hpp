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

#pragma once

#include <string>
#include <vector>
#include <optional>
#include "parser/lexeme.hpp"
#include "target/target.hpp"

namespace kdl::sema
{

    class component
    {
    public:
        enum class mode
        {
            import_file, export_lua_as_resource
        };

        struct file
        {
            std::string path;
            std::optional<std::string> name;

            explicit file(std::string path) : path(std::move(path)), name({}) {};
            file(std::string path, std::string name) : path(std::move(path)), name(std::move(name)) {};
        };

    public:
        component() = default;
        explicit component(std::string name, enum mode mode = mode::import_file);

        [[nodiscard]] auto name() const -> std::string;

        auto set_path_prefix(const std::string& path) -> void;
        [[nodiscard]] auto path_prefix() const -> std::string;

        auto set_namespace(const std::string& ns) -> void;
        [[nodiscard]] auto ns() const -> std::string;

        auto set_base_id(int64_t id) -> void;
        [[nodiscard]] auto base_id() const -> int64_t;

        auto set_type(const kdl::lexeme& type) -> void;
        [[nodiscard]] auto type() const -> kdl::lexeme;

        auto add_file(const std::string& path) -> void;
        auto add_file(const std::string& path, const std::string& name) -> void;
        auto set_files(std::vector<file> files) -> void;
        [[nodiscard]] auto files() const -> std::vector<file>;

        auto set_export_types(const std::vector<lexeme>& types) -> void;

        auto generate_resources(const std::shared_ptr<target>& target) const -> void;
        auto synthesize_lua_from_types(const std::shared_ptr<target>& target) const -> void;

    private:
        enum mode m_mode;
        std::string m_name { "Untitled Component" };
        bool m_scene { false };
        std::string m_path_prefix;
        std::string m_namespace;
        kdl::lexeme m_as_type { "", lexeme::identifier };
        std::int64_t m_base_id { 128 };
        std::vector<file> m_files;
        std::vector<lexeme> m_export_types;
    };

}
