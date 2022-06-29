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

#include <any>
#include <string>
#include "disassembler/kdl_exporter.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "parser/lexeme.hpp"
#include "target/target.hpp"
#include "disassembler/task.hpp"

namespace kdl::disassembler
{

    class resource_exporter
    {
    public:
        resource_exporter(task& task, kdl_exporter& exporter, build_target::type_container& type);

        auto disassemble(graphite::rsrc::resource *resource) -> void;

    private:
        graphite::rsrc::resource::identifier m_id;
        task& m_task;
        kdl_exporter& m_exporter;
        build_target::type_container& m_container;
        std::map<int, std::any> m_extracted_values;
        std::map<int, int> m_visited_template_fields;
        std::map<int, std::tuple<graphite::data::block, std::string>> m_file_exports;
        std::map<int, std::string> m_final_values;
        std::map<int, std::string> m_final_field_assoc;
        std::map<int, std::vector<std::string>> m_final_field_repeat;

        [[nodiscard]] auto find_substitutions(const build_target::type_field& field, const std::any& value) const -> std::vector<lexeme>;

        auto write_field_value(const build_target::type_template::binary_field& tmpl_field_info,
                               const std::any& extracted_value) -> std::string;

        auto extract_kdl_field(const build_target::type_field& field, int pass = 1) -> void;
        auto repeat_kdl_field_extraction(const build_target::type_field& field) -> void;

        auto export_kdl_field(const build_target::type_field& field, int pass = 1) -> void;
        auto repeat_kdl_field_export(const build_target::type_field& field) -> void;
    };

}
