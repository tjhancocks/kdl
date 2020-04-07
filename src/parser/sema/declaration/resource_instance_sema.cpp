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
#include "resource_instance_sema.hpp"
#include "field_sema.hpp"
#include "parser/file.hpp"
#include "target/resource.hpp"

auto kdl::sema::resource_instance_sema::test(kdl::sema::parser &parser) -> bool
{
    return parser.expect({
        expectation(lexeme::identifier, "new").be_true()
    });
}

auto kdl::sema::resource_instance_sema::parse(kdl::sema::parser &parser, kdl::container &type_container, std::weak_ptr<kdl::target> target, const bool is_example) -> void
{
    if (target.expired()) {
        throw std::logic_error("KDL Target is expired, and thus can not continue.");
    }
    auto t = target.lock();

    // Begin a new resource instance, and get the ID and Name of the resource if provided.
    auto resource_first_lx = parser.peek();
    parser.ensure({
        expectation(lexeme::identifier, "new").be_true(),
        expectation(lexeme::l_paren).be_true()
    });

    int64_t resource_id = INT64_MIN;
    std::string resource_name;

    if (parser.expect({
        expectation(lexeme::res_id).be_true(),
        expectation(lexeme::comma).be_true(),
        expectation(lexeme::string).be_true()
    })) {
        resource_id = std::stoll(parser.read().text());
        parser.advance();
        resource_name = parser.read().text();
    }
    else if (parser.expect({
        expectation(lexeme::res_id).be_true()
    })) {
        resource_id = parser.read().value<decltype(resource_id)>();
    }
    else if (parser.expect({
        expectation(lexeme::string).be_true()
    })) {
        resource_name = parser.read().text();
    }

    parser.ensure({ expectation(lexeme::r_paren).be_true() });

    // Begin the resource declaration block.
    auto resource_data = type_container.instantiate_resource(resource_id, resource_name);
    parser.ensure({ expectation(lexeme::l_brace).be_true() });

    while (parser.expect({ expectation(lexeme::r_brace).be_false() })) {

        if (field_sema::test(parser)) {
            field_sema::parse(parser, type_container, resource_data, target);
        }
        else {
            log::fatal_error(parser.peek(), 1, "Expected identifier for field name.");
        }

        parser.ensure({ expectation(lexeme::semi).be_true() });
    }

    parser.ensure({ expectation(lexeme::r_brace).be_true() });

    // Run any assertions that the type specifies on this instance.
    for (auto assertion : type_container.assertions()) {
        if (!assertion.evaluate(resource_data.synthesize_variables())) {
            log::fatal_error(resource_first_lx, 1, "Assertion Failed: " + assertion.failure_text());
        }
    }

    // Add the resource to the target.
    if (!is_example) {
        t->add_resource(resource_data);
    }
    else {
        // Assemble the resource so that any errors relating to that can trigger.
        resource_data.assemble();
    }
}
