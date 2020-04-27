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


#if !defined(KDL_RESOURCE_EXPORTER_HPP)
#define KDL_RESOURCE_EXPORTER_HPP

#include <any>
#include <string>
#include "disassembler/kdl_exporter.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "parser/lexeme.hpp"
#include "target/target.hpp"

namespace kdl { namespace disassembler {

    class resource_exporter
    {
    private:
        kdl_exporter& m_exporter;
        build_target::type_container& m_container;

        auto disassemble_value(build_target::binary_type base_type, build_target::type_field_value& expected_value,
                               std::any disasm_value) const -> std::string;

    public:
        resource_exporter(kdl_exporter& exporter, build_target::type_container& type);

        auto disassemble(std::shared_ptr<graphite::rsrc::resource> resource) -> void;
    };

}};

#endif //KDL_RESOURCE_EXPORTER_HPP
