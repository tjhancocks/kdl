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

#include "generation/lua/lua_generator.hpp"

// MARK: - Construction

kdl::generator::lua_type::lua_type(const kdl::build_target::type_container &type, const std::string& out_dir)
    : m_type(type), m_out_dir(out_dir)
{

}

// MARK: - Code Generation

auto kdl::generator::lua_type::generate() -> std::string
{
    comment(m_type.name() + ".lua automatically generated by KDL.");

    // Class Definition
    comment_divider();
    comment("Class Definition");
    define_type_class();

    // Constructor
    comment_divider();
    comment("Construction");
    standard_constructor();

    // Constants
    comment_divider();
    comment(m_type.name() + " Constants");

    define_function_constant("resourceTypeCode", m_type.code());

    // Properties / Fields
    comment_divider();
    comment(m_type.name() + " Fields");

    define_property("resourceId", false);
    define_property("resourceName", false);

    for (auto i = 0; i < m_type.internal_template().binary_field_count(); ++i) {
        auto field = m_type.internal_template().binary_field_at(i);
        define_property(field.label.text());
    }

    // Build a file and save it to disk.
    kdl::file file;
    file.set_contents(m_lua);
    file.save(m_out_dir + "/" + m_type.name() + ".lua");

    // Return the Lua code to the caller.
    return m_lua;
}

// MARK: - Syntax Generators

auto kdl::generator::lua_type::blank_line() -> void
{
    m_lua.append("\n");
}

auto kdl::generator::lua_type::comment_divider() -> void
{
    blank_line();
    m_lua.append("-- ");
    for (auto i = 0; i < 77; ++i) {
        m_lua.append("-");
    }
    m_lua.append("\n");
}

auto kdl::generator::lua_type::comment(const std::string &text) -> void
{
    m_lua.append("-- " + text + "\n");
}

auto kdl::generator::lua_type::define_type_class() -> void
{
    m_lua.append(m_type.name() + " = Class:new()\n");
}

auto kdl::generator::lua_type::define_constant(const std::string& name, const std::string& value) -> void
{
    auto constant_name = name;
    constant_name[0] = tolower(constant_name[0]);

    auto symbol = class_symbol(constant_name, "properties");
    m_lua.append(symbol + " = {}\n");
    begin_function("get", symbol);
    m_lua.append("   return \"" + value + "\"\n");
    end();
}

auto kdl::generator::lua_type::define_function_constant(const std::string& name, const std::string& value) -> void
{
    auto constant_name = name;
    constant_name[0] = tolower(constant_name[0]);

    begin_static_function(constant_name, m_type.name());
    m_lua.append("   return \"" + value + "\"\n");
    end();
}

auto kdl::generator::lua_type::define_property(const std::string& name, const bool& setter) -> void
{
    if (name == "_") {
        return;
    }

    auto property_name = name;
    property_name[0] = tolower(property_name[0]);

    blank_line();

    auto symbol = class_symbol(property_name, "properties");
    m_lua.append(symbol + " = {}\n");
    begin_function("get", symbol);
    m_lua.append("   return self._" + property_name + "\n");
    end();

    if (setter) {
        begin_function("set", symbol, { "value" });
        m_lua.append("   self._" + property_name + " = value\n");
        end();
    }
}

auto kdl::generator::lua_type::class_symbol(const std::string &name, const std::string& path) -> std::string
{
    return m_type.name() + "." + path + "." + name;
}

auto kdl::generator::lua_type::begin_function(const std::string &name, const std::string &super, const std::vector<std::string> args) -> void
{
    m_lua.append("function " + super + ":" + name);

    m_lua.append("(");
    if (!args.empty()) {
        for (auto i = 0; i < args.size(); ++i) {
            if (i > 0) {
                m_lua.append(", ");
            }
            m_lua.append(args[i]);
        }
    }
    m_lua.append(")\n");
}

auto kdl::generator::lua_type::begin_static_function(const std::string &name, const std::string &super, const std::vector<std::string> args) -> void
{
    m_lua.append("function " + super + "." + name);

    m_lua.append("(");
    if (!args.empty()) {
        for (auto i = 0; i < args.size(); ++i) {
            if (i > 0) {
                m_lua.append(", ");
            }
            m_lua.append(args[i]);
        }
    }
    m_lua.append(")\n");
}

auto kdl::generator::lua_type::end() -> void
{
    m_lua.append("end\n");
}

auto kdl::generator::lua_type::standard_constructor() -> void
{
    begin_static_function("load", m_type.name(), { "id" });

    // Find the resource first
    m_lua.append("    local res = " + m_type.name() + ":new()\n");
    m_lua.append("    res._resourceId = id\n");
    m_lua.append("    local resource = Resource.idWithType(" + m_type.name() + ".resourceTypeCode(), id)\n");
    m_lua.append("    local data = ResourceData(resource)\n");
    m_lua.append("    res._dataSuccessfullyLoaded = data.isValid\n");
    m_lua.append("    if not res._dataSuccessfullyLoaded then\n");
    m_lua.append("        print(\"" + m_type.code() + " is not valid: \" .. id)\n");
    m_lua.append("        return\n");
    m_lua.append("    end\n\n");

    for (auto i = 0; i < m_type.internal_template().binary_field_count(); ++i) {
        auto field = m_type.internal_template().binary_field_at(i);
        auto field_name = field.label.text();
        field_name[0] = tolower(field_name[0]);
        m_lua.append("    res._" + field_name + " = ");

        auto length = field.type & 0xFFF;
        switch (field.type & ~0xFFF) {
            case kdl::build_target::binary_type::DBYT:
                m_lua.append("data:readSignedByte()");
                break;
            case kdl::build_target::binary_type::DWRD:
                m_lua.append("data:readSignedShort()");
                break;
            case kdl::build_target::binary_type::DLNG:
                m_lua.append("data:readSignedLong()");
                break;
            case kdl::build_target::binary_type::DQAD:
                m_lua.append("data:readSignedQuad()");
                break;
            case kdl::build_target::binary_type::HBYT:
                m_lua.append("data:readByte()");
                break;
            case kdl::build_target::binary_type::HWRD:
                m_lua.append("data:readShort()");
                break;
            case kdl::build_target::binary_type::HLNG:
                m_lua.append("data:readLong()");
                break;
            case kdl::build_target::binary_type::HQAD:
                m_lua.append("data:readQuad()");
                break;
            case kdl::build_target::binary_type::RECT:
                m_lua.append("Rect(");
                m_lua.append("data:readSignedShort(), ");
                m_lua.append("data:readSignedShort(), ");
                m_lua.append("data:readSignedShort(), ");
                m_lua.append("data:readSignedShort()");
                m_lua.append(")");
                break;
            case kdl::build_target::binary_type::CSTR:
                m_lua.append("data:readCStr()");
                break;
            case kdl::build_target::binary_type::PSTR:
                m_lua.append("data:readPStr()");
                break;
            case kdl::build_target::binary_type::Cnnn:
                m_lua.append("data:readCStrOfLength(" + std::to_string(length) + ")");
                break;
            default:
                m_lua.append("nil");
                break;
        }

        m_lua.append("\n");
    }

    m_lua.append("    return res\n");

    end();
}
