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

#include <stdexcept>
#include <utility>
#include "diagnostic/fatal.hpp"
#include "parser/sema/util/list_parser.hpp"
#include "parser/sema/declarations/resource_instance_parser.hpp"
#include "parser/sema/declarations/field_parser.hpp"
#include "target/track/resource_importer.hpp"

// MARK: - Constructor

kdl::sema::resource_instance_parser::resource_instance_parser(kdl::sema::parser &parser,
                                                              kdl::build_target::type_container &type,
                                                              std::weak_ptr<target> target,
                                                              bool discards)
    : m_type(type), m_parser(parser), m_target(std::move(target)), m_discards(discards)
{

}

// MARK: - Configuration

auto kdl::sema::resource_instance_parser::set_keyword(const std::string& keyword) -> void
{
    m_keyword = keyword;
}

auto kdl::sema::resource_instance_parser::set_id(const int64_t& id) -> void
{
    m_id = id;
}

auto kdl::sema::resource_instance_parser::set_name(const std::string& name) -> void
{
    m_name = name;
}

auto kdl::sema::resource_instance_parser::set_attributes(const std::map<std::string, std::string> &attributes) -> void
{
    m_attributes = attributes;
}

auto kdl::sema::resource_instance_parser::add_attribute(const std::string& name, const std::string& value) -> void
{
    m_attributes[name] = value;
}

// MARK: - Parser

auto kdl::sema::resource_instance_parser::parse() -> kdl::build_target::resource_instance
{
    if (m_target.expired()) {
        throw std::logic_error("Expired target found in declaration parser.");
    }
    auto target = m_target.lock();

    auto first_lx = m_parser.peek();
    m_parser.ensure({
        expectation(lexeme::identifier, m_keyword).be_true()
    });

    // Setup a source id that can be used for duplication.
    auto source_id = INT64_MIN;

    if (m_keyword == "duplicate" && m_parser.expect({ expectation(lexeme::l_paren).be_true() })) {
        // The duplicate syntax is slightly unique and specific, and not just a simple list of values that can
        // be parsed.
        m_parser.advance();

        if (m_parser.expect({
            expectation(lexeme::res_id).be_true(),
            expectation(lexeme::identifier, "as").be_true(),
            expectation(lexeme::res_id).be_true()
        })) {
            source_id = m_parser.read().value<int64_t>();
            m_parser.advance();
            m_id = m_parser.read().value<int64_t>();
        }
        else {
            log::fatal_error(m_parser.peek(), 1, "Incorrect syntax for resource duplication.");
        }

        // We may have subsequent items here so we need to parse them out... check for a comma to see if further
        // items exist or not.
        if (m_parser.expect({ expectation(lexeme::comma).be_true() })) {
            m_parser.read();

            while (m_parser.expect({ expectation(lexeme::r_paren).be_false() })) {
                const auto &token = m_parser.read();

                if (token.is(lexeme::string)) {
                    m_name = token.text();
                }

                if (m_parser.expect({expectation(lexeme::r_paren).be_false()})) {
                    m_parser.ensure({expectation(lexeme::comma).be_true()});
                }
            }
        }

        m_parser.ensure({ expectation(lexeme::r_paren).be_true() });
    }
    else if (m_parser.expect({ expectation(lexeme::l_paren).be_true() })) {
        list_parser list(m_parser, m_target);

        list.set_list_start(lexeme::l_paren);
        list.set_list_end(lexeme::r_paren);
        list.set_delimiter(lexeme::comma);
        list.add_valid_list_item(lexeme::res_id);
        list.add_valid_list_item(lexeme::string);
        auto args = list.parse();

        for (const auto& arg : args) {
            if (arg.is(lexeme::res_id, "auto")) {
                m_id = INT64_MIN;
            }
            else if (arg.is(lexeme::res_id)) {
                m_id = arg.value<int64_t>();
            }
            else if (arg.is(lexeme::string)) {
                m_name = arg.text();
            }
        }

        // 'new' doesn't care about source_id, and 'override' requires it to be equal to m_id.
        source_id = m_id;
    }

    // If the ID of the resource is INT64_MIN, then we should automatically lookup a new resource id.
    if (m_id == INT64_MIN) {
        m_id = target->resource_tracker()->next_available_id(m_type.code());
    }

    auto instance = m_type.new_instance(m_id, m_name);

    // Is this resource one that is overriding another? If it is then we need to pre-populate the resource with the data
    // of the original (if it exists.)
    if (m_keyword == "override" || m_keyword == "duplicate") {
        // First check if the existing resource exists in our scope.
        auto tracker = target->resource_tracker();
        if (!tracker->instance_exists(m_type.code(), source_id)) {
            // The resource does not exist, so throw an error!
            log::fatal_error(first_lx, 1, "Attempting to "+ m_keyword + " resource '" + m_type.code() + "' #" + std::to_string(source_id) + ", but no existing resource found.");
        }

        // We can safely assume that the resource exists... load the resource from the resource manager and request that
        // it be parsed into something that we can use here.
        if (!kdl::resource_tracking::importer(m_type.code(), source_id).populate(instance, target->file())) {
            log::fatal_error(first_lx, 1, "Unable to "+ m_keyword + " resource '" + m_type.code() + "' #" + std::to_string(source_id));
        }
    }
    else {
        // Acquire a new instance of the resource and populate it with default values.
        for (const auto& field : m_type.all_fields()) {
            field_parser(m_parser, m_type, instance, m_target).apply_defaults_for_field(field.name());
        }
        m_parser.clear_pushed_lexemes();
    }
    instance.reset_acquisition_locks();

    m_parser.ensure({ expectation(lexeme::l_brace).be_true() });
    while (m_parser.expect({ expectation(lexeme::r_brace).be_false() })) {
        field_parser(m_parser, m_type, instance, m_target).parse();
        m_parser.ensure({ expectation(lexeme::semi).be_true() });
    }
    m_parser.ensure({ expectation(lexeme::r_brace).be_true() });

    // Before we can hand the instance back to the caller, we need to run any assertions on it to ensure it's
    // validity.
    for (const auto& assertion : m_type.assertions()) {
        if (!assertion.evaluate(instance.synthesize_variables())) {
            log::fatal_error(first_lx, 1, "Assertion Failed: " + assertion.failure_text());
        }
    }

    // Assign the attributes to the resource instance.
    for (const auto& attribute : m_attributes) {
        instance.set_attribute(attribute.first, attribute.second);
    }

    // Add the resource to the target now.
    target->add_resource(instance);

    return instance;
}
