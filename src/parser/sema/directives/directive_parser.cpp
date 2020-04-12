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

// MARK: - Constructor

kdl::sema::asm_directive::asm_directive(kdl::sema::parser &parser)
    : m_parser(parser)
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
        out_directive_parser::parse(m_parser);
    }
    else {
        log::fatal_error(directive, 1, "Unrecognised directive '" + directive.text() + "'");
    }
}