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

#include <string>
#include <vector>
#include <type_traits>
#include "parser/lexeme.hpp"
#include "target/new/type_container.hpp"

namespace kdl::codegen::lua::ast
{

    struct symbol;
    struct ast_node;
    struct block;
    struct class_definition;
    struct function_definition;
    struct property_definition;

    class generator
    {
    public:
        generator();
        ~generator();

        [[nodiscard]] auto generate_lua() const -> std::string;

        auto add_comment(const std::string& text) -> void;
        auto new_line() -> void;

        auto comma(ast_node *node) -> ast_node *;

        auto begin_block() -> void;
        auto else_block(ast_node *condition = nullptr) -> void;
        auto end_block() -> void;

        auto declare_class(const std::string& name, bool implicit = false) -> class_definition *;
        auto construct(class_definition *klass) -> ast_node *;

        auto declare_function(bool implicit, struct symbol* name, const std::vector<struct symbol *>& parameters = {}) -> function_definition *;
        auto declare_function(bool implicit, struct symbol* name, class_definition *klass, const std::vector<struct symbol *>& parameters = {}) -> function_definition *;
        auto declare_static_function(bool implicit, struct symbol *name, class_definition *klass, const std::vector<struct symbol *>& parameters = {}) -> function_definition *;

        auto declare_property(class_definition *klass, struct symbol *name, bool implicit = false) -> property_definition *;
        auto synthesize_getter(property_definition *property) -> ast_node *;
        auto synthesize_setter(property_definition *property) -> ast_node *;

        auto camel_case(const std::string& name) -> ast::symbol *;
        auto symbol(const std::string& name) -> ast::symbol *;
        auto private_symbol(const std::string& name) -> ast::symbol *;
        auto symbol(ast::symbol *symbol) -> ast::symbol *;
        auto private_symbol(ast::symbol *symbol) -> ast::symbol *;
        auto member(ast_node *member, ast_node *object = nullptr) -> ast_node *;
        auto apply_not(ast_node *expression) -> ast_node *;
        auto nil() -> ast_node *;
        auto self() -> ast_node *;
        auto number(int64_t n) -> ast_node *;
        auto string(const std::string& str) -> ast_node *;
        auto condition(ast_node *condition) -> ast_node *;
        auto return_statement(ast_node *expression) -> ast_node *;
        auto local_assign(struct symbol *var, ast_node *expression) -> ast_node *;
        auto assign(ast_node *lhs, ast_node *expression) -> ast_node *;
        auto call(function_definition *function, const std::vector<struct ast_node *>& arguments = {}) -> ast_node *;
        auto call(struct ast_node *expression, function_definition *function, const std::vector<struct ast_node *>& arguments = {}) -> ast_node *;
        auto userdata_literal() -> ast_node *;
        auto subscript(ast_node *value, ast_node *index) -> ast_node *;
        auto count(ast_node *value) -> ast_node *;
        auto add(ast_node *lhs, ast_node *rhs) -> ast_node *;

        auto push(ast::block *block) -> void;
        auto pop() -> void;

    private:
        std::vector<struct ast_node *> m_nodes;
        block *m_file { nullptr };
        block *m_scope { nullptr };
    };

    // -----

    struct ast_node
    {
    public:
        [[nodiscard]] virtual auto identifier() const -> std::string { return ""; }
        [[nodiscard]] virtual auto generate_lua(std::uint8_t indent) const -> std::vector<std::string> { return {""}; }
    };

    // -----

    struct comment: public ast_node
    {
    public:
        explicit comment(const std::string& text);
        [[nodiscard]] auto generate_lua(std::uint8_t indent) const -> std::vector<std::string> override;

    private:
        std::vector<std::string> m_lines;
    };

    // -----

    struct comma_terminator: public ast_node
    {
    public:
        explicit comma_terminator(ast_node *node);
        [[nodiscard]] auto generate_lua(std::uint8_t indent) const -> std::vector<std::string> override;

    private:
        ast_node *m_node { nullptr };
    };

    // -----

    struct symbol: public ast_node
    {
    public:
        explicit symbol(const std::string& name);
        [[nodiscard]] auto identifier() const -> std::string override;
        [[nodiscard]] auto generate_lua(std::uint8_t indent) const -> std::vector<std::string> override;

    private:
        std::string m_name;
    };

    // -----

    struct member: public ast_node
    {
    public:
        explicit member(ast_node *member, ast_node *object = nullptr);

        [[nodiscard]] auto generate_lua(std::uint8_t indent) const -> std::vector<std::string> override;

    private:
        ast_node *m_member { nullptr };
        ast_node *m_object { nullptr };
    };

    // -----

    struct block: public ast_node
    {
    public:
        explicit block(block *parent = nullptr);

        [[nodiscard]] auto is_root() const -> bool;
        [[nodiscard]] auto has_children() const -> bool;
        [[nodiscard]] auto parent() const -> block *;
        [[nodiscard]] auto else_condition() const -> bool;
        auto set_else_condition(bool f, ast_node *condition = nullptr) -> void;
        auto remove_end() -> void;

        auto add_node(ast_node *node) -> ast_node *;
        [[nodiscard]] auto generate_lua(std::uint8_t indent) const -> std::vector<std::string> override;

        auto adopt_parent(ast::block *parent) -> ast::block *;

    private:
        std::vector<ast_node *> m_nodes;
        block *m_parent { nullptr };
        bool m_else { false };
        bool m_endless { false };
        ast_node *m_else_condition { nullptr };
    };

    // -----

    struct function_definition: public ast_node
    {
    public:
        explicit function_definition(struct symbol *name, class_definition *klass = nullptr, bool member = true, const std::vector<struct symbol *>& parameters = {});

        [[nodiscard]] auto is_member() const -> bool;
        [[nodiscard]] auto parent_class() const -> class_definition *;

        [[nodiscard]] auto identifier() const -> std::string override;
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        class_definition *m_class { nullptr };
        bool m_member { true };
        struct symbol *m_name { nullptr };
        std::vector<struct symbol *> m_parameters;
    };

    // -----

    struct function_call: public ast_node
    {
    public:
        explicit function_call(function_definition *function, const std::vector<ast_node *>& arguments = {});
        function_call(struct ast_node *expression, function_definition *function, const std::vector<ast_node *>& arguments = {});

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string>;

    private:
        function_definition *m_function { nullptr };
        std::vector<ast_node *> m_arguments;
        struct ast_node *m_object_expression { nullptr };
    };

    // -----

    struct class_definition: public ast_node
    {
    public:
        explicit class_definition(const std::string& name);

        [[nodiscard]] auto identifier() const -> std::string override;
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        std::string m_name;
    };

    // -----

    struct self_literal: public ast_node
    {
    public:
        self_literal() = default;
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;
    };

    // -----

    struct property_definition: public ast_node
    {
    public:
        property_definition(class_definition *klass, struct symbol *name);
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

        [[nodiscard]] auto name() const -> struct symbol *;
        [[nodiscard]] auto path() const -> std::string;

    private:
        class_definition *m_class { nullptr };
        struct symbol *m_name { nullptr };
    };

    // -----

    struct property_accessor: public ast_node
    {
    public:
        explicit property_accessor(property_definition *property, bool getter = true);
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        property_definition *m_property { nullptr };
        bool m_getter { true };
    };

    // -----

    struct return_statement: public ast_node
    {
    public:
        explicit return_statement(ast_node *expression);

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        ast_node *m_expression { nullptr };
    };

    // -----

    struct assign_statement: public ast_node
    {
    public:
        assign_statement(ast_node *lhs, ast_node *expression);

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        ast_node *m_lhs { nullptr };
        ast_node *m_rhs { nullptr };
    };

    // -----

    struct local_assign_statement: public ast_node
    {
    public:
        local_assign_statement(symbol *var, ast_node *expression);

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        symbol *m_symbol { nullptr };
        ast_node *m_expression { nullptr };
    };

    // -----

    struct addition_expression: public ast_node
    {
    public:
        addition_expression(ast_node *lhs, ast_node *rhs);
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        ast_node *m_lhs { nullptr };
        ast_node *m_rhs { nullptr };
    };

    // -----

//    struct subtraction_expression;
//    struct multiply_expression;
//    struct division_expression;

    // -----

    struct subscript_expression: public ast_node
    {
    public:
        subscript_expression(ast_node *value, ast_node *key);
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        ast_node *m_value { nullptr };
        ast_node *m_key { nullptr };
    };

    // -----

    struct userdata_literal: public ast_node
    {
    public:
        explicit userdata_literal(ast::block *block = nullptr);
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

        [[nodiscard]] auto block() -> ast::block *;

    private:
        struct block *m_block { nullptr };
    };

    // -----

    struct count_operator: public ast_node
    {
    public:
        explicit count_operator(ast_node *value);
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        ast_node *m_value { nullptr };
    };

    // -----

    struct number_literal: public ast_node
    {
    public:
        explicit number_literal(int64_t n);

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        int64_t m_value;
    };

    // -----

    struct string_literal: public ast_node
    {
    public:
        explicit string_literal(const std::string& str);

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        std::string m_string;
    };

    // -----

//    struct boolean_literal;

    // -----

    struct not_expression: public ast_node
    {
    public:
        explicit not_expression(ast_node *expression);

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        ast_node *m_expression { nullptr };
    };

    // -----

//    struct and_expression;
//    struct or_expression;

    // -----

    struct if_condition: public ast_node
    {
    public:
        explicit if_condition(ast_node *condition);

        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;

    private:
        ast_node *m_condition { nullptr };
    };

    // -----

//    struct while_loop;
//    struct for_loop;
//    struct object_definition;

    // -----

    struct nil_literal: public ast_node
    {
    public:
        nil_literal() = default;
        [[nodiscard]] auto generate_lua(uint8_t indentation) const -> std::vector<std::string> override;
    };

}
