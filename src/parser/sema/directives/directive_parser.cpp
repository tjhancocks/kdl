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

#include "diagnostic/fatal.hpp"
#include "parser/sema/directives/directive_parser.hpp"
#include "parser/sema/directives/out_directive_parser.hpp"
#include "parser/sema/directives/import_directive_parser.hpp"
#include "parser/sema/directives/configuration_directive_parser.hpp"
#include "parser/sema/directives/require_directive_parser.hpp"
#include "parser/sema/directives/project_directive_parser.hpp"
#include "parser/sema/directives/author_directive_parser.hpp"
#include "parser/sema/directives/version_directive_parser.hpp"
#include "parser/sema/directives/const_directive_parser.hpp"
#include "parser/sema/directives/function_directive_parser.hpp"

// MARK: - Constructor

kdl::sema::asm_directive::asm_directive(kdl::sema::parser &parser, std::weak_ptr<target> target)
    : m_parser(parser), m_target(target)
{

}

// MARK: - Parser

auto kdl::sema::asm_directive::parse() -> void
{
    if (!m_parser.expect({ expectation(lexeme::directive).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "A '@' (directive) identifier expected");
    }
    auto directive = m_parser.read();

    if (directive.text() == "out") {
        out_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "import") {
        import_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "configuration") {
        configuration_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "require") {
        require_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "project") {
        project_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "author") {
        author_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "version") {
        version_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "const") {
        const_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "var") {
        // TODO: Make a mutable version of the parser, and make const actually constant.
        const_directive_parser::parse(m_parser, m_target);
    }
    else if (directive.text() == "function") {
        function_directive_parser::parse(m_parser, m_target);
    }
    else {
        log::fatal_error(directive, 1, "Unrecognised directive '" + directive.text() + "'");
    }
}