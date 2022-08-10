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

#pragma once

#include <functional>
#include "target/new/type_container.hpp"
#include "target/new/type_template.hpp"
#include "codegen/lua/ast.hpp"

namespace kdl::codegen::lua
{
    class type_exporter
    {
    public:
        explicit type_exporter(kdl::build_target::type_container& container);

        [[nodiscard]] auto generate_lua() -> std::string;

    private:
        kdl::build_target::type_container& m_container;
        ast::generator m_gen;

        struct {
            ast::class_definition *resource_reader { nullptr };
            ast::symbol *load_helper { nullptr };
            ast::function_definition *load_helper_function { nullptr };
            ast::function_definition *read_signed_byte { nullptr };
            ast::function_definition *read_signed_short { nullptr };
            ast::function_definition *read_signed_long { nullptr };
            ast::function_definition *read_signed_quad { nullptr };
            ast::function_definition *read_byte { nullptr };
            ast::function_definition *read_short { nullptr };
            ast::function_definition *read_long { nullptr };
            ast::function_definition *read_quad { nullptr };
            ast::function_definition *read_pstr { nullptr };
            ast::function_definition *read_cstr { nullptr };
            ast::function_definition *read_cstr_of_length { nullptr };
            ast::function_definition *read_color { nullptr };
            ast::function_definition *read_reference { nullptr };
            ast::function_definition *read_typed_reference { nullptr };
            ast::function_definition *read_macintosh_rect { nullptr };
            ast::function_definition *read_rect { nullptr };
            ast::function_definition *read_point { nullptr };
            ast::function_definition *read_size { nullptr };
            ast::function_definition *skip { nullptr };
            ast::class_definition *color_klass { nullptr };
            ast::function_definition *color_klass_color_value { nullptr };
            ast::class_definition *namespace_klass { nullptr };
            ast::function_definition *namespace_global { nullptr };
            ast::function_definition *identified_resource { nullptr };
        } m_kestrel_api;

        struct {
            ast::symbol *klass_name { nullptr };
            ast::class_definition *klass { nullptr };
            ast::function_definition *type_code_func { nullptr };
            std::unordered_map<std::string, ast::ast_node *> bin_fields;
        } m_type;

        auto load_kestrel_api() -> void;
        auto produce_header() -> void;
        auto produce_class_boilerplate() -> void;
        auto produce_type_constants() -> void;
        auto produce_model_loader() -> void;
        auto produce_type_properties() -> void;
        auto produce_property_getter(ast::property_definition *property) -> void;
        auto produce_property_setter(ast::property_definition *property) -> void;

        auto prepare_template_read_calls(ast::symbol *resource, ast::symbol *data) -> void;
        auto produce_template_read_calls(ast::symbol *resource) -> void;
        auto produce_template_property_mapping(ast::symbol *resource) -> void;

        auto produce_read_call(const build_target::type_template::binary_field& bin_field,
                               const build_target::type_field_value& field_value,
                               ast::symbol *data) -> ast::ast_node *;
        auto produce_read_call(const build_target::type_template::binary_field& bin_field,
                               ast::symbol *data) -> ast::ast_node *;


        auto build_field_name(build_target::type_field_value value, std::int32_t field_number = -1) -> ast::symbol *;
        auto field_for_binary_field(const build_target::type_template::binary_field& field) -> build_target::type_field;
        auto field_value_for_binary_field(const build_target::type_template::binary_field& field) -> build_target::type_field_value;
        auto associated_explicit_type_for_binary_field(const build_target::type_template::binary_field& field) -> std::optional<build_target::kdl_type>;

        auto produce_resource_reader_for_loop(ast::ast_node *var, ast::symbol *data,
                                              ast::ast_node *lower, ast::ast_node *upper,
                                              const std::function<auto(ast::ast_node *)->void>& block) -> void;


    };
}