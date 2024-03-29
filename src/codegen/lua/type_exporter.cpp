// Copyright (c) 2022 Tom Hancocks
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
#include <unordered_map>
#include "codegen/lua/type_exporter.hpp"
#include "codegen/lua/ast.hpp"
#include "diagnostic/fatal.hpp"

// MARK: - Construction

kdl::codegen::lua::type_exporter::type_exporter(kdl::build_target::type_container &container)
    : m_container(container)
{
    load_kestrel_api();
}

// MARK: - Helpers

auto kdl::codegen::lua::type_exporter::load_kestrel_api() -> void
{
    m_kestrel_api.resource_reader = m_gen.declare_class("ResourceReader", true);
    m_kestrel_api.load_helper = m_gen.symbol("kdl_resourceLoader");
    m_kestrel_api.load_helper_function = m_gen.declare_function(true, m_kestrel_api.load_helper);
    m_kestrel_api.read_signed_byte = m_gen.declare_function(true, m_gen.symbol("readSignedByte"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_signed_short = m_gen.declare_function(true, m_gen.symbol("readSignedShort"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_signed_long = m_gen.declare_function(true, m_gen.symbol("readSignedLong"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_signed_quad = m_gen.declare_function(true, m_gen.symbol("readSignedQuad"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_byte = m_gen.declare_function(true, m_gen.symbol("readByte"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_short = m_gen.declare_function(true, m_gen.symbol("readShort"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_long = m_gen.declare_function(true, m_gen.symbol("readLong"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_quad = m_gen.declare_function(true, m_gen.symbol("readQuad"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_pstr = m_gen.declare_function(true, m_gen.symbol("readPStr"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_cstr = m_gen.declare_function(true, m_gen.symbol("readCStr"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_cstr_of_length = m_gen.declare_function(true, m_gen.symbol("readCStrOfLength"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_color = m_gen.declare_function(true, m_gen.symbol("readColor"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_reference = m_gen.declare_function(true, m_gen.symbol("readResourceReference"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_typed_reference = m_gen.declare_function(true, m_gen.symbol("readTypedResourceReference"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_macintosh_rect = m_gen.declare_function(true, m_gen.symbol("readMacintoshRect"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_rect = m_gen.declare_function(true, m_gen.symbol("readRect"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_point = m_gen.declare_function(true, m_gen.symbol("readPoint"), m_kestrel_api.resource_reader);
    m_kestrel_api.read_size = m_gen.declare_function(true, m_gen.symbol("readSize"), m_kestrel_api.resource_reader);
    m_kestrel_api.skip = m_gen.declare_function(true, m_gen.symbol("skip"), m_kestrel_api.resource_reader);

    m_kestrel_api.color_klass = m_gen.declare_class("Color", true);
    m_kestrel_api.color_klass_color_value = m_gen.declare_static_function(true, m_gen.symbol("colorValue"), m_kestrel_api.color_klass);

    m_kestrel_api.namespace_klass = m_gen.declare_class("Namespace", true);
    m_kestrel_api.namespace_global = m_gen.declare_static_function(true, m_gen.symbol("global"), m_kestrel_api.namespace_klass);
    m_kestrel_api.identified_resource = m_gen.declare_function(true, m_gen.symbol("identifiedResource"), m_kestrel_api.namespace_klass);
}

auto kdl::codegen::lua::type_exporter::determine_export_names() -> void
{
    for (const auto& field : m_container.all_fields()) {
        for (auto i = 0; i < field.expected_values(); ++i) {
            auto value = field.value_at(i);
            if (value.export_name().has_value()) {
                m_type.export_names.emplace(std::pair(value.base_name().text(), value.export_name()->text()));
            }

            for (auto j = 0; j < value.joined_value_count(); ++j) {
                auto joined_value = value.joined_value_at(j);
                if (joined_value.export_name().has_value()) {
                    m_type.export_names.emplace(std::pair(joined_value.base_name().text(), joined_value.export_name()->text()));
                }
            }
        }
    }
}

auto kdl::codegen::lua::type_exporter::produce_header() -> void
{
    m_gen.add_comment("This code was auto generated by KDL.");
    m_gen.add_comment("Do not modify, as alterations will be automatically reverted.");
    m_gen.new_line();
}

auto kdl::codegen::lua::type_exporter::produce_class_boilerplate() -> void
{
    m_gen.add_comment(m_container.name() + " Class Definition");
    m_type.klass_name = m_gen.symbol(m_container.name());
    m_type.klass = m_gen.declare_class(m_container.name());
    m_gen.new_line();

    auto resource_type_code_sym = m_gen.symbol("resourceTypeCode");
    m_type.type_code_func = m_gen.declare_static_function(false, resource_type_code_sym, m_type.klass);
    m_gen.begin_block();
        m_gen.return_statement(m_gen.string(m_container.code()));
    m_gen.end_block();
    m_gen.new_line();
}

auto kdl::codegen::lua::type_exporter::produce_model_loader() -> void
{
    auto ref = m_gen.symbol("resourceReference");

    m_gen.add_comment(" Construction");
    m_gen.declare_static_function(false, m_gen.symbol("load"), m_type.klass, { ref });
    m_gen.begin_block();
    {
        m_gen.condition(m_gen.apply_not(ref));
        m_gen.begin_block();
        {
            m_gen.return_statement(m_gen.nil());
        }
        m_gen.end_block();
        m_gen.new_line();

        auto resource = m_gen.symbol("resource");
        auto data = m_gen.symbol("data");

        m_gen.local_assign(resource, m_gen.construct(m_type.klass));
        m_gen.local_assign(data, m_gen.call(m_kestrel_api.load_helper_function, { resource, ref, m_gen.call(m_type.type_code_func) }));
        m_gen.new_line();

        m_gen.condition(m_gen.apply_not(data));
        m_gen.begin_block();
        {
            m_gen.return_statement(m_gen.nil());
        }
        m_gen.end_block();
        m_gen.new_line();

        prepare_template_read_calls(resource, data);
        produce_template_read_calls(resource);
        produce_template_property_mapping(resource);

        m_gen.return_statement(resource);
    }
    m_gen.end_block();
}

auto kdl::codegen::lua::type_exporter::build_field_name(build_target::type_field_value value, std::int32_t field_number) -> ast::symbol *
{
    if (value.export_name().has_value()) {
        return m_gen.camel_case(value.export_name()->text());
    }
    else if (field_number >= 0) {
        std::unordered_map<std::string, lexeme> vars{
            std::pair("FieldNumber", lexeme(std::to_string(field_number), lexeme::integer))
        };
        return m_gen.camel_case(value.extended_name(vars).text());
    }
    else {
        return m_gen.camel_case(value.base_name().text());
    }
}

auto kdl::codegen::lua::type_exporter::field_for_binary_field(const build_target::type_template::binary_field &field) -> build_target::type_field
{
    auto tmpl = m_container.internal_template();

    for (const auto& container_field : m_container.all_fields()) {

        auto lower = container_field.lower_repeat_bound();
        auto upper = container_field.has_repeatable_count_field() ? lower : container_field.upper_repeat_bound();

        if (container_field.has_repeatable_count_field() && container_field.repeatable_count_field().is(field.label.text())) {
            return container_field;
        }

        for (auto n = lower; n <= upper; ++n) {
            for (auto i = 0; i < container_field.expected_values(); ++i) {
                auto value = container_field.value_at(i);
                auto name = value.extended_name({
                    std::pair("FieldNumber", lexeme(std::to_string(n), lexeme::integer))
                });

                if (field.label.is(name.text())) {
                    return container_field;
                }

                for (auto j = 0; j < value.joined_value_count(); ++j) {
                    auto joined = value.joined_value_at(j);
                    auto joined_name = joined.extended_name({
                        std::pair("FieldNumber", lexeme(std::to_string(n), lexeme::integer))
                    });

                    if (field.label.is(joined_name.text())) {
                        return container_field;
                    }
                }
            }
        }
    }
    log::fatal_error(field.label, 1, "Binary field reference could not be found in field values.");
}

auto kdl::codegen::lua::type_exporter::field_value_for_binary_field(const build_target::type_template::binary_field &field) -> build_target::type_field_value
{
    auto tmpl = m_container.internal_template();

    for (const auto& container_field : m_container.all_fields()) {

        auto lower = container_field.lower_repeat_bound();
        auto upper = container_field.has_repeatable_count_field() ? lower : container_field.upper_repeat_bound();

        for (auto n = lower; n <= upper; ++n) {
            for (auto i = 0; i < container_field.expected_values(); ++i) {
                auto value = container_field.value_at(i);
                auto name = value.extended_name({
                    std::pair("FieldNumber", lexeme(std::to_string(n), lexeme::integer))
                });

                if (field.label.is(name.text())) {
                    return value;
                }

                for (auto j = 0; j < value.joined_value_count(); ++j) {
                    auto joined = value.joined_value_at(j);
                    auto joined_name = joined.extended_name({
                        std::pair("FieldNumber", lexeme(std::to_string(n), lexeme::integer))
                    });

                    if (field.label.is(joined_name.text())) {
                        return joined;
                    }
                }
            }
        }
    }
    log::fatal_error(field.label, 1, "Binary field reference could not be found in field values.");
}

auto kdl::codegen::lua::type_exporter::prepare_template_read_calls(ast::symbol *resource, ast::symbol *data) -> void
{
   auto tmpl = m_container.internal_template();

   // The first task for the resource reader is to produce all the read calls for the binary template fields.
   for (const auto& field : m_container.all_fields()) {
       if (field.is_repeatable() && field.has_repeatable_count_field()) {
           // Get the OCNT field for the count field, and add a reader for it.
           auto binary_field = tmpl.binary_field_named(field.repeatable_count_field().text());
           auto list_count = produce_read_call(binary_field, data);
           m_type.bin_fields.emplace(std::pair(binary_field.label.text(), list_count));

           for (auto i = 0; i < field.expected_values(); ++i) {
               auto value = field.value_at(i);

               binary_field = tmpl.binary_field_named(value.base_name());
               auto value_reader = produce_read_call(binary_field, value, data);
               m_type.bin_fields.emplace(std::pair(binary_field.label.text(), value_reader));

               if (value.joined_value_count() > 0) {
                   for (auto j = 0; j < value.joined_value_count(); ++j) {
                       auto joined_value = value.joined_value_at(j);
                       binary_field = tmpl.binary_field_named(joined_value.base_name());
                       value_reader = produce_read_call(binary_field, joined_value, data);
                       m_type.bin_fields.emplace(std::pair(binary_field.label.text(), value_reader));
                   }
               }
           }
       }
       else if (field.is_repeatable()) {
           for (auto i = field.lower_repeat_bound(); i <= field.upper_repeat_bound(); ++i) {
               for (auto j = 0; j < field.expected_values(); ++j) {
                   auto value = field.value_at(j);
                   auto binary_field = tmpl.binary_field_named(value.extended_name({
                       std::pair("FieldNumber", lexeme(std::to_string(i), lexeme::integer))
                   }));
                   auto value_reader = produce_read_call(binary_field, value, data);
                   m_type.bin_fields.emplace(std::pair(binary_field.label.text(), value_reader));
               }
           }
       }
       else {
           for (auto i = 0; i < field.expected_values(); ++i) {
               auto value = field.value_at(i);
               auto binary_field = tmpl.binary_field_named(value.base_name());
               auto value_reader = produce_read_call(binary_field, value, data);
               m_type.bin_fields.emplace(std::pair(binary_field.label.text(), value_reader));

               if (value.joined_value_count() > 0) {
                   for (auto j = 0; j < value.joined_value_count(); ++j) {
                       auto joined_value = value.joined_value_at(j);
                       binary_field = tmpl.binary_field_named(joined_value.base_name());
                       value_reader = produce_read_call(binary_field, joined_value, data);
                       m_type.bin_fields.emplace(std::pair(binary_field.label.text(), value_reader));
                   }
               }
           }
       }
   }
}


auto kdl::codegen::lua::type_exporter::produce_template_read_calls(ast::symbol *resource) -> void
{
    auto tmpl = m_container.internal_template();
    for (auto i = 0; i < tmpl.binary_field_count(); ++i) {
        auto bin_field = tmpl.binary_field_at(i);
        auto it = m_type.bin_fields.find(bin_field.label.text());
        if (it == m_type.bin_fields.end()) {
            auto skip_length = build_target::binary_type_base_size(bin_field.type);
            m_gen.call(m_gen.symbol("data"), m_kestrel_api.skip, { m_gen.number(skip_length) });
            continue;
        }

        auto resource_member = m_gen.member(m_gen.private_symbol(m_gen.camel_case(bin_field.label.text())), resource);

        if (bin_field.list_fields.empty()) {
            m_gen.assign(resource_member, it->second);
        }
        else {
            m_gen.new_line();

            auto count = m_gen.symbol(m_gen.camel_case(bin_field.label.text() + "Count"));
            m_gen.local_assign(count, it->second);

            auto idx = m_gen.symbol("idx");

            auto userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
            m_gen.assign(resource_member, userdata);

            m_gen.for_loop(idx, m_gen.number(1), count);
            m_gen.begin_block();
            {
                auto element = m_gen.subscript(resource_member, idx);
                auto element_userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
                m_gen.assign(element, element_userdata);
                m_gen.push(element_userdata->block());
                for (auto& list_field : bin_field.list_fields) {
                    auto it = m_type.bin_fields.find(list_field.label.text());
                    if (it == m_type.bin_fields.end()) {
                        log::fatal_error(bin_field.label, 2, "Failed to find corresponding binary field specified whilst exporting Lua.");
                    }

                    auto field_name = list_field.label.text();
                    auto name_it = m_type.export_names.find(list_field.label.text());
                    if (name_it != m_type.export_names.end()) {
                        field_name = name_it->second;
                    }

                    m_gen.assign(m_gen.camel_case(field_name), m_gen.comma(it->second));
                }
                m_gen.pop();
            }
            m_gen.pop();
            m_gen.new_line();
        }

    }
}

auto kdl::codegen::lua::type_exporter::produce_template_property_mapping(ast::symbol *resource) -> void
{
    m_gen.new_line();

    for (const auto& field : m_container.all_fields()) {
        if (field.is_repeatable() && !field.has_repeatable_count_field()) {
            auto var = m_gen.private_symbol(m_gen.camel_case(field.name().text()));
            auto userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
            m_gen.assign(m_gen.member(var, resource), userdata);

            m_gen.push(userdata->block());
            for (auto i = field.lower_repeat_bound(); i <= field.upper_repeat_bound(); ++i) {
                auto subscript = m_gen.subscript(nullptr, m_gen.number(i));
                ast::ast_node *element_value = nullptr;

                if (field.expected_values() == 1) {
                    auto value_name = field.value_at(0).extended_name({
                        std::pair("FieldNumber", lexeme(std::to_string(i), lexeme::integer))
                    });

                    element_value = m_gen.member(m_gen.private_symbol(m_gen.camel_case(value_name.text())), resource);
                }
                else {
                    auto element_userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
                    element_value = element_userdata;
                    m_gen.push(element_userdata->block());
                    {
                        for (auto j = 0; j < field.expected_values(); ++j) {
                            auto field_value = field.value_at(j);
                            auto var_name = build_field_name(field_value, i);
                            auto value_name = field_value.extended_name({
                                std::pair("FieldNumber", lexeme(std::to_string(i), lexeme::integer))
                            });
                            auto value_var = m_gen.member(m_gen.private_symbol(m_gen.camel_case(value_name.text())), resource);
                            m_gen.assign(var_name, m_gen.comma(value_var));
                        }
                    }
                    m_gen.pop();
                }

                m_gen.assign(subscript, m_gen.comma(element_value));
            }
            m_gen.pop();
        }
        else if (field.expected_values() > 1 && !field.is_repeatable()) {
            auto element_userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
            m_gen.push(element_userdata->block());
            {
                for (auto i = 0; i < field.expected_values(); ++i) {
                    auto field_value = field.value_at(i);
                    auto value_var = m_gen.member(m_gen.private_symbol(m_gen.camel_case(field_value.base_name().text())), resource);
                    m_gen.assign(build_field_name(field_value), m_gen.comma(value_var));
                }
            }
            m_gen.pop();
            auto var = m_gen.member(m_gen.private_symbol(m_gen.camel_case(field.name().text())), resource);
            m_gen.assign(var, element_userdata);
        }
        else if (field.expected_values() == 1 && field.value_at(0).joined_value_count() > 0) {
            auto element_userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
            m_gen.push(element_userdata->block());
            {
                auto field_value = field.value_at(0);
                auto value_var = m_gen.member(m_gen.private_symbol(m_gen.camel_case(field_value.base_name().text())), resource);
                m_gen.assign(build_field_name(field_value), m_gen.comma(value_var));

                for (auto i = 0; i < field_value.joined_value_count(); ++i) {
                    auto joined_value = field_value.joined_value_at(i);
                    value_var = m_gen.member(m_gen.private_symbol(m_gen.camel_case(joined_value.base_name().text())), resource);
                    m_gen.assign(build_field_name(joined_value), m_gen.comma(value_var));
                }
            }
            m_gen.pop();
            auto var = m_gen.member(m_gen.private_symbol(m_gen.camel_case(field.name().text())), resource);
            m_gen.assign(var, element_userdata);
        }
    }

    m_gen.new_line();
}

auto kdl::codegen::lua::type_exporter::produce_read_call(const build_target::type_template::binary_field& bin_field,
                                                         const build_target::type_field_value& field_value,
                                                         ast::symbol *data) -> ast::ast_node *
{
    ast::ast_node *value = nullptr;

    if (field_value.explicit_type().has_value()) {
        auto value_type = field_value.explicit_type().value();
        auto type = bin_field.type & ~0xFFF;
        auto length = bin_field.type & 0xFFF;

        if (value_type.is_reference() && value_type.name().has_value()) {
            auto klass = m_gen.declare_class(value_type.name()->text(), true);
            auto klass_type_code = m_gen.declare_static_function(true, m_gen.symbol("resourceTypeCode"), klass);
            value = m_gen.call(data, m_kestrel_api.read_typed_reference, { m_gen.call(klass_type_code) });
        }
        else if (value_type.is_reference()) {
            value = m_gen.call(data, m_kestrel_api.read_reference);
        }
        else if (value_type.name().has_value()) {
            if (value_type.name()->is("Color")) {
                value = m_gen.call(data, m_kestrel_api.read_color);
            }
            else if (value_type.name()->is("File")) {
                if (type == build_target::PSTR) {
                    value = m_gen.call(data, m_kestrel_api.read_pstr);
                }
                else if (type == build_target::CSTR) {
                    value = m_gen.call(data, m_kestrel_api.read_cstr);
                }
                else if (type == build_target::Cnnn) {
                    value = m_gen.call(data, m_kestrel_api.read_cstr_of_length, { m_gen.number(length) });
                }
            }
            else if (value_type.name()->is("Bitmask") || value_type.name()->is("Range")) {
                switch (type) {
                    case build_target::DBYT: value = m_gen.call(data, m_kestrel_api.read_signed_byte); break;
                    case build_target::DWRD: value = m_gen.call(data, m_kestrel_api.read_signed_short); break;
                    case build_target::DLNG: value = m_gen.call(data, m_kestrel_api.read_signed_long); break;
                    case build_target::DQAD: value = m_gen.call(data, m_kestrel_api.read_signed_quad); break;
                    case build_target::HBYT: value = m_gen.call(data, m_kestrel_api.read_byte); break;
                    case build_target::HWRD: value = m_gen.call(data, m_kestrel_api.read_short); break;
                    case build_target::HLNG: value = m_gen.call(data, m_kestrel_api.read_long); break;
                    case build_target::HQAD: value = m_gen.call(data, m_kestrel_api.read_quad); break;
                    default: value = m_gen.call(data, m_kestrel_api.read_short); break;
                }
            }
        }
    }
    else {
        value = produce_read_call(bin_field, data);
    }

    return value;
}

auto kdl::codegen::lua::type_exporter::produce_read_call(const build_target::type_template::binary_field& bin_field,
                                                         ast::symbol *data) -> ast::ast_node *
{
    ast::ast_node *value = nullptr;
    auto type = bin_field.type & ~0xFFF;
    auto length = bin_field.type & 0xFFF;

    switch (type) {
        case build_target::DBYT: value = m_gen.call(data, m_kestrel_api.read_signed_byte); break;
        case build_target::DWRD: value = m_gen.call(data, m_kestrel_api.read_signed_short); break;
        case build_target::DLNG: value = m_gen.call(data, m_kestrel_api.read_signed_long); break;
        case build_target::DQAD: value = m_gen.call(data, m_kestrel_api.read_signed_quad); break;
        case build_target::HBYT: value = m_gen.call(data, m_kestrel_api.read_byte); break;
        case build_target::HWRD: value = m_gen.call(data, m_kestrel_api.read_short); break;
        case build_target::HLNG: value = m_gen.call(data, m_kestrel_api.read_long); break;
        case build_target::HQAD: value = m_gen.call(data, m_kestrel_api.read_quad); break;
        case build_target::CSTR: value = m_gen.call(data, m_kestrel_api.read_cstr); break;
        case build_target::PSTR: value = m_gen.call(data, m_kestrel_api.read_pstr); break;
        case build_target::RECT: value = m_gen.call(data, m_kestrel_api.read_rect); break;
        case build_target::OCNT: value = m_gen.call(data, m_kestrel_api.read_short); break;
        case build_target::Cnnn: {
            value = m_gen.call(data, m_kestrel_api.read_cstr_of_length, { m_gen.number(length) });
            break;
        }
        default: {
            value = m_gen.nil();
            break;
        }
    }

    return value;
}


auto kdl::codegen::lua::type_exporter::produce_resource_reader_for_loop(ast::ast_node *var, ast::symbol *data,
                                                                        ast::ast_node *lower, ast::ast_node *upper,
                                                                        const std::function<auto(ast::ast_node *)->void>& block) -> void
{
    auto idx = m_gen.symbol("idx");

    m_gen.for_loop(idx, lower, upper);
    m_gen.begin_block();
    {
        auto userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
        m_gen.assign(m_gen.subscript(var, idx), userdata);
        m_gen.push(userdata->block());
        block(idx);
        m_gen.pop();
    }
    m_gen.end_block();
    m_gen.new_line();
}

auto kdl::codegen::lua::type_exporter::associated_explicit_type_for_binary_field(const build_target::type_template::binary_field &field) -> std::optional<build_target::kdl_type>
{
    for (const auto& container_field : m_container.all_fields()) {
        for (auto n = container_field.lower_repeat_bound(); n <= container_field.upper_repeat_bound(); ++n) {
            for (auto i = 0; i < container_field.expected_values(); ++i) {
                auto value = container_field.value_at(i);
                auto name = value.extended_name({ std::pair("FieldNumber", lexeme(std::to_string(n), lexeme::integer)) });
                auto bin_field = m_container.internal_template().binary_field_named(name);
                if (bin_field.label.text() == field.label.text()) {
                    return value.explicit_type();
                }
            }
        }
    }
    return {};
}

auto kdl::codegen::lua::type_exporter::produce_type_constants() -> void
{
    m_gen.add_comment("Constants");

    auto produce_constants_for_value = [&] (const build_target::type_field_value& value, std::int32_t lower_bound) {
        if (value.symbols().empty()) {
            return;
        }

        auto bin_field = m_container.internal_template().binary_field_named(value.extended_name({
            std::pair("FieldNumber", lexeme(std::to_string(lower_bound), lexeme::integer))
        }));

        auto constants_userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
        m_gen.assign(m_gen.symbol(value.base_name().text()), m_gen.comma(constants_userdata));
        m_gen.push(constants_userdata->block());
        {
            for (auto& symbol : value.symbols()) {
                auto symbol_name = std::get<0>(symbol);
                auto symbol_value = std::get<1>(symbol);

                ast::ast_node *constant = m_gen.nil();
                if (value.explicit_type().has_value()) {
                    if (value.explicit_type()->is_reference()) {
                        auto global = m_gen.call(m_kestrel_api.namespace_global);
                        constant = m_gen.call(global, m_kestrel_api.identified_resource, {
                            m_gen.number(symbol_value.value<std::int64_t>())
                        });
                    }
                    else if (value.explicit_type()->name().has_value()) {
                        if (value.explicit_type()->name()->is("Color")) {
                            constant = m_gen.call(m_kestrel_api.color_klass_color_value, { m_gen.number(symbol_value.value<std::uint32_t>()) });
                        }
                        else if (value.explicit_type()->name()->is("Bitmask") || value.explicit_type()->name()->is("Range")) {
                            constant = m_gen.number(symbol_value.value<std::int64_t>());
                        }
                        else if (value.explicit_type()->name()->is("File")) {
                            constant = m_gen.string(symbol_value.text());
                        }
                    }
                }
                else {
                    switch (bin_field.type) {
                        case build_target::DBYT:
                        case build_target::DWRD:
                        case build_target::DLNG:
                        case build_target::DQAD:
                        case build_target::HBYT:
                        case build_target::HWRD:
                        case build_target::HLNG:
                        case build_target::HQAD:
                            constant = m_gen.number(symbol_value.value<std::int64_t>());
                            break;
                        case build_target::PSTR:
                        case build_target::CSTR:
                            constant = m_gen.string(symbol_value.text());
                            break;
                        default:
                            break;
                    }
                }

                m_gen.assign(m_gen.symbol(symbol_name.text()), m_gen.comma(constant));
            }
        }
        m_gen.pop();
    };

    auto userdata = reinterpret_cast<ast::userdata_literal *>(m_gen.userdata_literal());
    m_gen.assign(m_gen.member(m_gen.symbol("Constants"), m_type.klass_name), userdata);
    m_gen.push(userdata->block());
    {
        for (const auto& container_field : m_container.all_fields()) {
            for (auto i = 0; i < container_field.expected_values(); ++i) {
                auto value = container_field.value_at(i);
                produce_constants_for_value(value, container_field.lower_repeat_bound());

                if (value.joined_value_count() > 0) {
                    for (auto j = 0; j < value.joined_value_count(); ++j) {
                        produce_constants_for_value(value.joined_value_at(j), container_field.lower_repeat_bound());
                    }
                }
            }
        }
    }
    m_gen.pop();

    m_gen.new_line();
}

auto kdl::codegen::lua::type_exporter::produce_type_properties() -> void
{
    m_gen.new_line();
    m_gen.add_comment("Properties");

    for (const auto& field : m_container.all_fields()) {
        auto name = m_gen.camel_case(field.name().text());
        auto symbol = name;

        if (field.has_repeatable_count_field()) {
            symbol = m_gen.camel_case(field.repeatable_count_field().text());
        }

        auto property = m_gen.declare_property(m_type.klass, name, symbol);

        produce_property_getter(property);
        if (field.wants_lua_setter()) {
            produce_property_setter(property);
        }

        m_gen.new_line();
    }
}

auto kdl::codegen::lua::type_exporter::produce_property_getter(ast::property_definition *property) -> void
{
    m_gen.synthesize_getter(property);
}

auto kdl::codegen::lua::type_exporter::produce_property_setter(ast::property_definition *property) -> void
{
    m_gen.synthesize_setter(property);
}

// MARK: - Code Generator

auto kdl::codegen::lua::type_exporter::generate_lua() -> std::string
{
    determine_export_names();
    produce_header();
    produce_class_boilerplate();
    produce_type_constants();
    produce_model_loader();
    produce_type_properties();

//    std::cout << m_gen.generate_lua() << std::endl;

    return m_gen.generate_lua();
}
