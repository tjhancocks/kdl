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

#include <fstream>
#include "disassembler/kdl_exporter.hpp"

// MARK: - Construction

kdl::disassembler::kdl_exporter::kdl_exporter(const std::string &path)
    : m_path(path)
{
    m_dir = path;
    while (!m_dir.empty()) {
        if (m_dir.back() == '/') {
            m_dir.pop_back();
            break;
        }
        m_dir.pop_back();
    }
}

// MARK: - File

auto kdl::disassembler::kdl_exporter::save() -> void
{
    std::ofstream out(m_path);
    out << m_code;
}

auto kdl::disassembler::kdl_exporter::export_file(const std::string &name, const std::string& contents) -> void
{
    std::ofstream out(m_dir + "/" + name);
    out << contents;
}

auto kdl::disassembler::kdl_exporter::export_file(const std::string &name, const std::vector<char>& contents) -> void
{
    std::ofstream out(m_dir + "/" + name, std::ios::out | std::ios::binary);
    out.write((char*)&contents[0], contents.size());
    out.close();
}

// MARK: - Code Generation

auto kdl::disassembler::kdl_exporter::insert_line(const std::string &line, int indent) -> void
{
    for (auto i = 0; i < indent; ++i) {
        m_code.append("    ");
    }
    m_code.append(line + "\n");
}

auto kdl::disassembler::kdl_exporter::insert_comment(const std::string &text) -> void
{
    insert_line("` " + text, 0);
}

auto kdl::disassembler::kdl_exporter::begin_declaration(const std::string &name) -> void
{
    insert_line("declare " + name + " {");
}

auto kdl::disassembler::kdl_exporter::end_declaration() -> void
{
    insert_line("};");
}

auto kdl::disassembler::kdl_exporter::begin_resource(int64_t id, const std::string &name) -> void
{
    if (name.empty()) {
        insert_line("new (#" + std::to_string(id) + ") {", 1);
    }
    else {
        insert_line("new (#" + std::to_string(id) + ", \"" + escape_strings(name) + "\") {", 1);
    }
}

auto kdl::disassembler::kdl_exporter::end_resource() -> void
{
    insert_line("};\n", 1);
}

auto kdl::disassembler::kdl_exporter::add_field(const std::string &name, std::vector<std::string> values) -> void
{
    std::string line { name + " = " };
    for (auto value : values) {
        line.append(value + " ");
    }
    insert_line(line + ";", 2);
}
