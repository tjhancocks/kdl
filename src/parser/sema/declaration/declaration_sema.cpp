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

#include "diagnostic/fatal.hpp"
#include "declaration_sema.hpp"
#include "resource_instance_sema.hpp"

auto kdl::sema::declaration_sema::test(kdl::sema::parser& parser) -> bool
{
    return parser.expect({
        expectation(lexeme::identifier, "declare").be_true(),
        expectation(lexeme::identifier).be_true()
    });
}

auto kdl::sema::declaration_sema::parse(kdl::sema::parser& parser, std::weak_ptr<kdl::target> target, const bool is_example) -> void
{
    if (target.expired()) {
        throw std::logic_error("KDL Target is expired, and thus can not continue.");
    }
    auto t = target.lock();

    parser.ensure({ expectation(lexeme::identifier, "declare").be_true() });

    // Get the type container for the declaration block.
    if (!parser.expect({ expectation(lexeme::identifier).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Resource declaration block type must be an identifier.");
    }
    auto declaration_type = parser.read();
    auto type_container = t->container_named(declaration_type);

    parser.ensure({ expectation(lexeme::l_brace).be_true() });

    while (parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        if (resource_instance_sema::test(parser)) {
            resource_instance_sema::parse(parser, type_container, target, is_example);
        }
        else {
            log::fatal_error(parser.peek(), 1, "Expected either an 'override' or a 'new' keyword.");
        }

        parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    parser.ensure({
        expectation(lexeme::r_brace).be_true(),
        expectation(lexeme::semi).be_true()
    });
}


