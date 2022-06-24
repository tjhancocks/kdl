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

#pragma once

#include <string>
#include <libGraphite/rsrc/resource.hpp>
#include "parser/lexeme.hpp"

namespace kdl::disassembler
{

    class kdl_exporter
    {
    public:
        explicit kdl_exporter(const std::string& path);

        auto save() -> void;
        auto export_file(const std::string& name, const std::string& contents) -> void;
        auto export_file(const std::string& name, const std::vector<char>& contents) -> void;
        auto export_file(const std::string& name, const graphite::data::block& data) -> void;

        auto insert_comment(const std::string& text) -> void;

        auto begin_declaration(const std::string& name) -> void;
        auto end_declaration() -> void;

        auto begin_resource(graphite::rsrc::resource::identifier id, const std::string& name) -> void;
        auto end_resource() -> void;

        auto add_field(const std::string& name, std::vector<std::string> values) -> void;

    private:
        std::string m_path;
        std::string m_dir;
        std::string m_code { "" };

        auto insert_line(const std::string& line, int indent = 0) -> void;

    };

}

// MARK: - Helpers

static auto escape_strings(const std::string& str) -> std::string
{
    std::string out;
    for (auto i = str.begin(); i != str.end(); ++i) {
        if ((*i) == '"') {
            out.append("\\");
        }
        out.insert(out.end(), i, i + 1);
    }
    return out;
}

