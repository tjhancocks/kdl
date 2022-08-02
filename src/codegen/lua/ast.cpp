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

#include <sstream>
#include "codegen/lua/ast.hpp"

// MARK: - Indentation

static inline auto indent_line(const std::string& input, uint8_t size) -> std::string
{
    std::string indentation_str = "    ";
    std::string result = input;
    for (auto i = 0; i < size; ++i) {
        result.insert(result.begin(), indentation_str.begin(), indentation_str.end());
    }
    return result;
}

// MARK: - Generator

kdl::codegen::lua::ast::generator::generator()
{
    m_file = new block();
    m_scope = m_file;
}

kdl::codegen::lua::ast::generator::~generator()
{
    delete m_file;
    for (auto it : m_nodes) {
        delete it;
    }
}

auto kdl::codegen::lua::ast::generator::generate_lua() const -> std::string
{
    std::string result;
    for (auto it : m_file->generate_lua(0)) {
        result += it + "\n";
    }
    return result;
}

auto kdl::codegen::lua::ast::generator::new_line() -> void
{
    auto node = m_scope->add_node(new ast_node());
    m_nodes.emplace_back(node);
}

// MARK: - Comment

auto kdl::codegen::lua::ast::generator::add_comment(const std::string &text) -> void
{
    auto node = m_scope->add_node(new comment(text));
    m_nodes.emplace_back(node);
}

kdl::codegen::lua::ast::comment::comment(const std::string &text)
{
    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line, '\n')) {
        m_lines.emplace_back(line);
    }
}

auto kdl::codegen::lua::ast::generator::emit(ast_node *node) -> void
{
    m_scope->add_node(node);
}

auto kdl::codegen::lua::ast::comment::generate_lua(std::uint8_t indentation) const -> std::vector<std::string>
{
    std::vector<std::string> result;
    for (const auto& line : m_lines) {
        result.emplace_back(indent_line("-- " + line, indentation));
    }
    return result;
}

// MARK: - Symbol

kdl::codegen::lua::ast::symbol::symbol(const std::string &name)
    : m_name(name)
{
}

auto kdl::codegen::lua::ast::symbol::identifier() const -> std::string
{
    return m_name;
}

auto kdl::codegen::lua::ast::symbol::generate_lua(std::uint8_t indent) const -> std::vector<std::string>
{
    return { indent_line(identifier(), indent) };
}

auto kdl::codegen::lua::ast::generator::camel_case(const std::string &name) -> ast::symbol *
{
    auto camel_case = name;
    camel_case[0] = std::tolower(camel_case[0]);

    auto node = new ast::symbol(camel_case);
    m_nodes.emplace_back(node);
    return reinterpret_cast<ast::symbol *>(node);
}

auto kdl::codegen::lua::ast::generator::symbol(const std::string &name) -> ast::symbol *
{
    auto node = new ast::symbol(name);
    m_nodes.emplace_back(node);
    return reinterpret_cast<ast::symbol *>(node);
}

auto kdl::codegen::lua::ast::generator::private_symbol(const std::string &name) -> ast::symbol *
{
    auto node = new ast::symbol("_" + name);
    m_nodes.emplace_back(node);
    return reinterpret_cast<ast::symbol *>(node);
}

auto kdl::codegen::lua::ast::generator::symbol(ast::symbol *name) -> ast::symbol *
{
    auto node = new ast::symbol(name->identifier());
    m_nodes.emplace_back(node);
    return reinterpret_cast<ast::symbol *>(node);
}

auto kdl::codegen::lua::ast::generator::private_symbol(ast::symbol *name) -> ast::symbol *
{
    auto node = new ast::symbol("_" + name->identifier());
    m_nodes.emplace_back(node);
    return reinterpret_cast<ast::symbol *>(node);
}

// MARK: - Block

kdl::codegen::lua::ast::block::block(block *parent)
    : m_parent(parent)
{
}

auto kdl::codegen::lua::ast::block::is_root() const -> bool
{
    return m_parent == nullptr;
}

auto kdl::codegen::lua::ast::block::has_children() const -> bool
{
    return !m_nodes.empty();
}

auto kdl::codegen::lua::ast::block::parent() const -> block *
{
    return m_parent;
}

auto kdl::codegen::lua::ast::block::else_condition() const -> bool
{
    return m_else;
}

auto kdl::codegen::lua::ast::block::set_else_condition(bool f, ast_node *condition) -> void
{
    m_else = f;
    m_else_condition = condition;
}

auto kdl::codegen::lua::ast::block::remove_end() -> void
{
    m_endless = true;
}

auto kdl::codegen::lua::ast::block::add_node(ast_node *node) -> ast_node *
{
    m_nodes.emplace_back(node);
    return node;
}

auto kdl::codegen::lua::ast::block::generate_lua(std::uint8_t indentation) const -> std::vector<std::string>
{
    std::vector<std::string> result;
    uint8_t indent = indentation + 1;
    if (is_root()) {
        indent = 0;
    }

    for (const auto it : m_nodes) {
        auto node_out = it->generate_lua(indent);
        result.insert(result.end(), node_out.begin(), node_out.end());
    }

    if (m_parent && !m_endless) {
        if (m_else && m_else_condition) {
            auto condition = m_else_condition->generate_lua(0).front();
            result.emplace_back(indent_line("else" + condition, indentation));
        }
        else if (m_else) {
            result.emplace_back(indent_line("else", indentation));
        }
        else {
            result.emplace_back(indent_line("end", indentation));
        }
    }

    return result;
}

auto kdl::codegen::lua::ast::generator::begin_block() -> void
{
    auto node = m_scope->add_node(new block(m_scope));
    m_scope = reinterpret_cast<decltype(m_scope)>(node);
    m_nodes.emplace_back(node);
}

auto kdl::codegen::lua::ast::generator::end_block() -> void
{
    if (m_scope->is_root()) {
        return;
    }
    m_scope = m_scope->parent();
}

auto kdl::codegen::lua::ast::generator::else_block(ast_node *condition) -> void
{
    if (m_scope->is_root()) {
        return;
    }
    m_scope->set_else_condition(true, condition);
    m_scope = m_scope->parent();
}

auto kdl::codegen::lua::ast::block::adopt_parent(ast::block *parent) -> ast::block *
{
    m_parent = parent;
    return this;
}

// MARK: - Class Definition

auto kdl::codegen::lua::ast::generator::declare_class(const std::string &name, bool implicit) -> class_definition *
{
    auto node = new class_definition(name);

    if (!implicit) {
        m_scope->add_node(node);
    }

    m_nodes.emplace_back(node);
    return reinterpret_cast<class_definition *>(node);
}

kdl::codegen::lua::ast::class_definition::class_definition(const std::string &name)
    : m_name(name)
{
}

auto kdl::codegen::lua::ast::class_definition::identifier() const -> std::string
{
    return m_name;
}

auto kdl::codegen::lua::ast::class_definition::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    symbol new_symbol("new");
    class_definition klass("Class");
    function_definition class_new(&new_symbol, &klass, true);

    symbol lhs(m_name);
    function_call rhs(&class_new);

    assign_statement stmt(&lhs, &rhs);
    return stmt.generate_lua(indentation);
}

auto kdl::codegen::lua::ast::generator::construct(class_definition *klass) -> ast_node *
{
    auto new_symbol = new ast::symbol("new");
    auto klass_new = new function_definition(new_symbol, klass, true);
    auto call = new function_call(klass_new);

    m_nodes.emplace_back(new_symbol);
    m_nodes.emplace_back(klass_new);
    m_nodes.emplace_back(call);

    return call;
}

// MARK: - Function Definition

kdl::codegen::lua::ast::function_definition::function_definition(struct symbol *name, class_definition *klass, bool member, const std::vector<struct symbol *> &parameters)
    : m_name(name), m_class(klass), m_member(member), m_parameters(parameters)
{
}

auto kdl::codegen::lua::ast::function_definition::identifier() const -> std::string
{
    return m_name->identifier();
}

auto kdl::codegen::lua::ast::function_definition::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    std::string result;

    result = "function ";

    if (m_class) {
        result += m_class->identifier();
        result += m_member ? ":" : ".";
    }

    result += m_name->identifier() + "(";
    for (auto it = m_parameters.begin(); it != m_parameters.end(); ++it) {
        if (it != m_parameters.begin()) {
            result += ", ";
        }
        result += const_cast<struct symbol *>(*it)->identifier();
    }
    result += ")";

    return {
        indent_line(result, indentation)
    };
}

auto kdl::codegen::lua::ast::function_definition::is_member() const -> bool
{
    return m_member;
}

auto kdl::codegen::lua::ast::function_definition::parent_class() const -> class_definition *
{
    return m_class;
}

auto kdl::codegen::lua::ast::generator::declare_function(bool implicit, struct symbol *name, const std::vector<struct symbol *> &parameters) -> function_definition *
{
    auto node = new function_definition(name, nullptr, false, parameters);

    if (!implicit) {
        m_scope->add_node(node);
    }

    m_nodes.emplace_back(node);
    return reinterpret_cast<function_definition *>(node);
}

auto kdl::codegen::lua::ast::generator::declare_function(bool implicit, struct symbol *name, class_definition *klass, const std::vector<struct symbol *> &parameters) -> function_definition *
{
    auto node = new function_definition(name, klass, true, parameters);

    if (!implicit) {
        m_scope->add_node(node);
    }

    m_nodes.emplace_back(node);
    return reinterpret_cast<function_definition *>(node);
}

auto kdl::codegen::lua::ast::generator::declare_static_function(bool implicit, struct symbol *name, class_definition *klass, const std::vector<struct symbol *>& parameters) -> function_definition *
{
    auto node = new function_definition(name, klass, false, parameters);

    if (!implicit) {
        m_scope->add_node(node);
    }

    m_nodes.emplace_back(node);
    return reinterpret_cast<function_definition *>(node);
}

// MARK: - Function Call

kdl::codegen::lua::ast::function_call::function_call(function_definition *function, const std::vector<ast_node *> &arguments)
    : m_function(function), m_arguments(arguments)
{
}

kdl::codegen::lua::ast::function_call::function_call(struct ast_node *expression, function_definition *function, const std::vector<ast_node *> &arguments)
    : m_function(function), m_arguments(arguments), m_object_expression(expression)
{
}

auto kdl::codegen::lua::ast::function_call::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    std::string result;

    if (m_function->parent_class()) {
        if (m_object_expression && m_function->is_member()) {
            result += m_object_expression->generate_lua(0).front();
        }
        else {
            result += m_function->parent_class()->identifier();
        }

        result += m_function->is_member() ? ":" : ".";
    }

    result += m_function->identifier() + "(";
    for (auto it = m_arguments.begin(); it != m_arguments.end(); ++it) {
        if (it != m_arguments.begin()) {
            result += ", ";
        }
        result += const_cast<struct ast_node *>(*it)->generate_lua(0).front();
    }
    result += ")";

    return { indent_line(result, indentation) };
}

auto kdl::codegen::lua::ast::generator::call(function_definition *function, const std::vector<struct ast_node *> &arguments) -> ast_node *
{
    auto node = new function_call(function, arguments);
    m_nodes.emplace_back(node);
    return reinterpret_cast<function_definition *>(node);
}

auto kdl::codegen::lua::ast::generator::call(struct ast_node *expression, function_definition *function, const std::vector<struct ast_node *> &arguments) -> ast_node *
{
    auto node = new function_call(expression, function, arguments);
    m_nodes.emplace_back(node);
    return reinterpret_cast<function_definition *>(node);
}

// MARK: - Assign Statement

kdl::codegen::lua::ast::assign_statement::assign_statement(ast_node *lhs, ast_node *expression)
    : m_lhs(lhs), m_rhs(expression)
{
}

auto kdl::codegen::lua::ast::assign_statement::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    std::vector<std::string> result;

    std::string lhs = m_lhs->generate_lua(0).front();
    auto rhs = m_rhs->generate_lua(indentation + 1);

    auto first_start = rhs.front().find_first_not_of(' ');
    auto first = rhs.front().substr(first_start == std::string::npos ? 0 : first_start);
    rhs.erase(rhs.begin());

    result.emplace_back(indent_line(lhs + " = " + first, indentation));
    for (const auto& it : rhs) {
        result.emplace_back(it);
    }

    return result;
}

auto kdl::codegen::lua::ast::generator::assign(ast_node *lhs, ast_node *expression) -> ast_node *
{
    auto node = m_scope->add_node(new assign_statement(lhs, expression));
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Not Expression

kdl::codegen::lua::ast::not_expression::not_expression(ast_node *expression)
    : m_expression(expression)
{}

auto kdl::codegen::lua::ast::not_expression::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    auto expr = m_expression->generate_lua(0).front();
    return {
        indent_line("not " + expr, indentation)
    };
}

auto kdl::codegen::lua::ast::generator::apply_not(ast_node *expression) -> ast_node *
{
    auto node = new not_expression(expression);
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - If Condition

kdl::codegen::lua::ast::if_condition::if_condition(ast_node *condition)
    : m_condition(condition)
{}

auto kdl::codegen::lua::ast::if_condition::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    auto expr = m_condition->generate_lua(0).front();
    return {
        indent_line("if " + expr + " then", indentation)
    };
}

auto kdl::codegen::lua::ast::generator::condition(ast_node *condition) -> ast_node *
{
    auto node = m_scope->add_node(new if_condition(condition));
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Nil Literal

auto kdl::codegen::lua::ast::nil_literal::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    return { "nil" };
}

auto kdl::codegen::lua::ast::generator::nil() -> ast_node *
{
    auto node = new nil_literal();
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Self Literal

auto kdl::codegen::lua::ast::self_literal::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    return { "self" };
}

auto kdl::codegen::lua::ast::generator::self() -> ast_node *
{
    auto node = new self_literal();
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Return Statement

kdl::codegen::lua::ast::return_statement::return_statement(ast_node *expression)
    : m_expression(expression)
{}

auto kdl::codegen::lua::ast::return_statement::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    auto expr = m_expression->generate_lua(0).front();
    return {
        indent_line("return " + expr, indentation)
    };
}

auto kdl::codegen::lua::ast::generator::return_statement(ast_node *expression) -> ast_node *
{
    auto node = m_scope->add_node(new struct return_statement(expression));
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Local Assign

kdl::codegen::lua::ast::local_assign_statement::local_assign_statement(symbol *var, ast_node *expression)
    : m_symbol(var), m_expression(expression)
{}

auto kdl::codegen::lua::ast::local_assign_statement::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    auto var = m_symbol->generate_lua(0).front();
    auto expr = m_expression->generate_lua(0).front();
    return {
        indent_line("local " + var + " = " + expr, indentation)
    };
}

auto kdl::codegen::lua::ast::generator::local_assign(struct symbol *var, ast_node *expression) -> ast_node *
{
    auto node = m_scope->add_node(new local_assign_statement(var, expression));
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - String Literal

kdl::codegen::lua::ast::string_literal::string_literal(const std::string& string)
    : m_string(string)
{}

auto kdl::codegen::lua::ast::string_literal::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    return { indent_line("\"" + m_string + "\"", indentation) };
}

auto kdl::codegen::lua::ast::generator::string(const std::string& string) -> ast_node *
{
    auto node = new string_literal(string);
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Member

kdl::codegen::lua::ast::member::member(ast_node *member, ast_node *object)
    : m_object(object), m_member(member)
{}

auto kdl::codegen::lua::ast::member::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    std::string result;

    if (m_object) {
        result += m_object->generate_lua(0).front() + ".";
    }
    result += m_member->generate_lua(0).front();

    return { result };
}

auto kdl::codegen::lua::ast::generator::member(ast_node *member, ast_node *object) -> ast_node *
{
    auto node = new struct member(member, object);
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Property Definition

kdl::codegen::lua::ast::property_definition::property_definition(class_definition *klass, struct symbol *name)
    : m_class(klass), m_name(name)
{}

auto kdl::codegen::lua::ast::property_definition::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    return { indent_line(path() + " = {}", indentation) };
}

auto kdl::codegen::lua::ast::property_definition::name() const -> struct symbol *
{
    return m_name;
}

auto kdl::codegen::lua::ast::property_definition::path() const -> std::string
{
    auto klass = m_class->identifier();
    auto name = m_name->identifier();
    return klass + ".properties." + name;
}

auto kdl::codegen::lua::ast::generator::declare_property(class_definition *klass, struct symbol *name, bool implicit) -> property_definition *
{
    auto node = new property_definition(klass, name);

    if (!implicit) {
        m_scope->add_node(node);
    }

    m_nodes.emplace_back(node);
    return reinterpret_cast<property_definition *>(node);
}

// MARK: - Property Definition

kdl::codegen::lua::ast::property_accessor::property_accessor(property_definition *property, bool getter)
    : m_property(property), m_getter(getter)
{}

auto kdl::codegen::lua::ast::property_accessor::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    std::vector<std::string> result;

    symbol value("newValue");
    std::vector<struct symbol *> params;
    if (!m_getter) {
        params.emplace_back(&value);
    }

    symbol name(m_getter ? "get" : "set");
    class_definition shim(m_property->path());
    function_definition accessor(&name, &shim, true, params);

    result.emplace_back(accessor.generate_lua(indentation).front());

    if (m_getter) {
        self_literal self;
        symbol property_value("_" + m_property->name()->identifier());
        member member_value(&property_value, &self);
        return_statement ret(&member_value);
        result.emplace_back(ret.generate_lua(indentation + 1).front());
    }
    else {
        self_literal self;
        symbol property_value("_" + m_property->name()->identifier());
        member member_value(&property_value, &self);
        assign_statement assign(&member_value, &value);
        result.emplace_back(assign.generate_lua(indentation + 1).front());
    }

    result.emplace_back(indent_line("end", indentation));
    return result;
}

auto kdl::codegen::lua::ast::generator::synthesize_getter(property_definition *property) -> ast_node *
{
    auto node = m_scope->add_node(new property_accessor(property, true));
    m_nodes.emplace_back(node);
    return node;
}

auto kdl::codegen::lua::ast::generator::synthesize_setter(property_definition *property) -> ast_node *
{
    auto node = m_scope->add_node(new property_accessor(property, false));
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Array Literal

kdl::codegen::lua::ast::userdata_literal::userdata_literal(ast::block *block)
    : m_block(block)
{}

auto kdl::codegen::lua::ast::userdata_literal::block() -> ast::block *
{
    return m_block;
}

auto kdl::codegen::lua::ast::userdata_literal::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    std::vector<std::string> result;
    if (m_block && m_block->has_children()) {
        m_block->remove_end();
        result.emplace_back(indent_line("{", indentation - 1));

        auto children = m_block->generate_lua(indentation - 1);
        for (const auto& child : children) {
            result.emplace_back(child);
        }

        result.emplace_back(indent_line("}", indentation - 1));
    }
    else {
        result.emplace_back(indent_line("{}", indentation - 1));
    }
    return result;
}

auto kdl::codegen::lua::ast::generator::userdata_literal() -> ast_node *
{
    auto block = new ast::block();
    auto node = new struct userdata_literal(block);
    m_nodes.emplace_back(block);
    m_nodes.emplace_back(node);
    return node;
}

auto kdl::codegen::lua::ast::generator::push(ast::block *block) -> void
{
    m_scope = block->adopt_parent(m_scope);
}

auto kdl::codegen::lua::ast::generator::pop() -> void
{
    if (m_scope->is_root()) {
        return;
    }

    m_scope = m_scope->parent();
}

// MARK: - Subscript Expression

kdl::codegen::lua::ast::subscript_expression::subscript_expression(ast_node *value, ast_node *key)
    : m_value(value), m_key(key)
{}

auto kdl::codegen::lua::ast::subscript_expression::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    auto value = m_value->generate_lua(0).front();
    auto key = m_key->generate_lua(0).front();
    return { value + "[" + key + "]" };
}

auto kdl::codegen::lua::ast::generator::subscript(ast_node *value, ast_node *index) -> ast_node *
{
    auto node = new struct subscript_expression(value, index);
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Number Literal

kdl::codegen::lua::ast::number_literal::number_literal(int64_t n)
    : m_value(n)
{}

auto kdl::codegen::lua::ast::number_literal::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    return { std::to_string(m_value) };
}

auto kdl::codegen::lua::ast::generator::number(int64_t n) -> ast_node *
{
    auto node = new struct number_literal(n);
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Count Operator

kdl::codegen::lua::ast::count_operator::count_operator(ast_node *value)
    : m_value(value)
{}

auto kdl::codegen::lua::ast::count_operator::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    auto value = m_value->generate_lua(0).front();
    return { "#" + value };
}

auto kdl::codegen::lua::ast::generator::count(ast_node *value) -> ast_node *
{
    auto node = new struct count_operator(value);
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Addition

kdl::codegen::lua::ast::addition_expression::addition_expression(ast_node *lhs, ast_node *rhs)
    : m_lhs(lhs), m_rhs(rhs)
{}

auto kdl::codegen::lua::ast::addition_expression::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    auto lhs = m_lhs->generate_lua(0).front();
    auto rhs = m_rhs->generate_lua(0).front();
    return { lhs + " + " + rhs };
}

auto kdl::codegen::lua::ast::generator::add(ast_node *lhs, ast_node *rhs) -> ast_node *
{
    auto node = new struct addition_expression(lhs, rhs);
    m_nodes.emplace_back(node);
    return node;
}

// MARK: - Comma

kdl::codegen::lua::ast::comma_terminator::comma_terminator(ast_node *node)
    : m_node(node)
{}

auto kdl::codegen::lua::ast::comma_terminator::generate_lua(uint8_t indentation) const -> std::vector<std::string>
{
    std::string comma = ",";
    auto result = m_node->generate_lua(indentation);
    result.back().insert(result.back().end(), comma.begin(), comma.end());
    return result;
}

auto kdl::codegen::lua::ast::generator::comma(ast_node *node) -> ast_node *
{
    auto new_node = new struct comma_terminator(node);
    m_nodes.emplace_back(new_node);
    return new_node;
}
