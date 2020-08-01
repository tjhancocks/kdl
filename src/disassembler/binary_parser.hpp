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


#if !defined(KDL_BINARY_PARSER_HPP)
#define KDL_BINARY_PARSER_HPP

#include <map>
#include <any>
#include "target/new/type_template.hpp"
#include "libGraphite/data/reader.hpp"

namespace kdl { namespace disassembler {

    class binary_parser
    {
    private:
        build_target::type_template& m_tmpl;
        int m_index { 0 };

        auto extract_value(graphite::data::reader& reader) -> std::any;

    public:
        binary_parser(build_target::type_template& tmpl);

        auto parse(graphite::data::reader& reader) -> std::map<int, std::any>;
    };

}}

#endif //KDL_BINARY_PARSER_HPP
