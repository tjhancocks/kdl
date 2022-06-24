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

#include <iostream>
#include <libGraphite/data/reader.hpp>
#include "parser/file.hpp"
#include "analyzer/template_extractor.hpp"

// MARK: - Constructor

kdl::analyzer::template_extractor::template_extractor(const std::string& in, const std::string& out)
    : m_in(in), m_out_dir(out)
{
    if (m_out_dir.substr(m_out_dir.size() - 1) == "/") {
        m_out_dir.pop_back();
    }
}

// MARK: - Type Definition Builder

auto kdl::analyzer::template_extractor::build_type_definitions() -> void
{
    if (auto tmpl = const_cast<graphite::rsrc::type *>(m_in.type("TMPL"))) {
        for (auto& res : *tmpl) {
            auto code = res.name();
            auto data = graphite::data::reader(&res.data());

            // Build the KDL Code for the template.
            std::string kdl_code;
            kdl_code.append("` Type Definition and Template generated from TMPL resource via KDL.\n\n");
            kdl_code.append("@type " + code + " : \"" + code + "\" {\n");
            kdl_code.append("    template {\n");

            while (!data.eof()) {
                auto label = data.read_pstr();
                auto type = data.read_cstr(4);
                kdl_code.append("        " + type + " " + label + ";\n");
            }

            kdl_code.append("    };\n");
            kdl_code.append("};\n");

            // Build a file and export it.
            kdl::file file;
            file.set_contents(kdl_code);
            file.save(m_out_dir + "/" + code + ".kdl");
        }
    }
    else {
        // Generate a warning that no TMPL resources were found.
    }
}

