// Copyright (c) 2021 Tom Hancocks
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

#include <utility>
#include "diagnostic/fatal.hpp"
#include "parser/sema/component/component_parser.hpp"
#include "parser/sema/component/file_list_parser.hpp"
#include "parser/sema/component/component_file.hpp"

// MARK: - Constructor

kdl::sema::component_parser::component_parser(kdl::sema::parser &parser, std::weak_ptr<target> target)
    : m_parser(parser), m_target(std::move(target)), m_component()
{

}
// MARK: - Parser

auto kdl::sema::component_parser::parse() -> void
{
    if (m_target.expired()) {
        throw std::logic_error("Expired target found in component parser.");
    }
    auto target = m_target.lock();

    m_parser.ensure({ expectation(lexeme::identifier, "component").be_true() });
    if (!m_parser.expect({ expectation(lexeme::string).be_true() })) {
        log::fatal_error(m_parser.peek(), 1, "Component name must be a string.");
    }
    auto component_name = m_parser.read();
    m_component = component(component_name.text());

    m_parser.ensure({ expectation(lexeme::l_brace).be_true() });
    while (m_parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        if (m_parser.expect({ expectation(lexeme::identifier, "path_prefix").be_true() })) {
            m_parser.advance();
            m_parser.ensure({expectation(lexeme::equals).be_true()});

            if (!m_parser.expect({expectation(lexeme::string).be_true()})) {
                log::fatal_error(m_parser.peek(), 1, "'path_prefix' requires a string to be provided.");
            }
            auto path = m_parser.read();
            m_component.set_path_prefix(path.text());
        }
        else if (m_parser.expect({ expectation(lexeme::identifier, "namespace").be_true() })) {
            m_parser.advance();
            m_parser.ensure({ expectation(lexeme::equals).be_true() });

            if (!m_parser.expect({expectation(lexeme::string).be_true()})) {
                log::fatal_error(m_parser.peek(), 1, "'namespace' requires a string to be provided.");
            }
            auto ns = m_parser.read();
            m_component.set_namespace(ns.text());
        }
        else if (m_parser.expect({ expectation(lexeme::identifier, "base_id").be_true() })) {
            m_parser.advance();
            m_parser.ensure({ expectation(lexeme::equals).be_true() });

            if (!m_parser.expect({expectation(lexeme::res_id).be_true()})) {
                log::fatal_error(m_parser.peek(), 1, "'base_id' requires a resource id to be provided.");
            }
            auto base_id = m_parser.read();
            m_component.set_base_id(base_id.value<int64_t>());
        }
        else if (m_parser.expect({ expectation(lexeme::identifier, "as_type").be_true() })) {
            m_parser.advance();
            m_parser.ensure({ expectation(lexeme::equals).be_true() });

            if (!m_parser.expect({expectation(lexeme::identifier).be_true()})) {
                log::fatal_error(m_parser.peek(), 1, "'as_type' requires a resource type to be provided.");
            }
            auto as_type = m_parser.read();
            m_component.set_type(as_type);
        }
        else if (m_parser.expect({ expectation(lexeme::identifier, "is_scene").be_true() })) {
            m_parser.advance();
        }
        else if (m_parser.expect({ expectation(lexeme::identifier, "files").be_true() })) {
            m_parser.advance();
            auto files = file_list_parser(m_parser, m_target).parse();
            m_component.set_files(files);
        }
        else {
            log::fatal_error(m_parser.peek(), 1, "Expected either 'path_prefix', 'namespace', 'is_scene', 'base_id' or 'files'.");
        }

        m_parser.ensure({ expectation(lexeme::semi).be_true() });
    }
    m_parser.ensure({ expectation(lexeme::r_brace).be_true() });

    // At this point we need to actually do something with the component.
    m_component.generate_resources(target);
}