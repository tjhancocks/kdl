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

#if !defined(KDL_LUA_GENERATOR_HPP)
#define KDL_LUA_GENERATOR_HPP

#include <string>
#include "target/new/type_container.hpp"

namespace kdl { namespace generator {

    class lua_type
    {
    private:
        std::string m_out_dir;
        std::string m_lua;
        build_target::type_container m_type;

        auto class_symbol(const std::string& name, const std::string& path = "") -> std::string;

        auto blank_line() -> void;
        auto comment_divider() -> void;
        auto comment(const std::string& text) -> void;
        auto define_type_class() -> void;

        auto define_function_constant(const std::string& name, const std::string& value) -> void;
        auto define_constant(const std::string& name, const std::string& value) -> void;
        auto define_property(const std::string& name, const bool& setter = true) -> void;

        auto begin_function(const std::string& name, const std::string& super, const std::vector<std::string> args = {}) -> void;
        auto begin_static_function(const std::string &name, const std::string &super, const std::vector<std::string> args = {}) -> void;
        auto end() -> void;

        auto standard_constructor() -> void;

    public:
        explicit lua_type(const build_target::type_container& type, const std::string& out_dir);

        auto generate() -> std::string;
    };

}};

#endif //KDL_LUA_GENERATOR_HPP
