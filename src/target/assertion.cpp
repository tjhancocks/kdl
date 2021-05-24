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
#include "target/assertion.hpp"

// MARK: - Constructor

kdl::assertion::assertion(const lexeme lhs, assertion::operation op, const lexeme rhs)
    : m_lhs(lhs), m_operation(op), m_rhs(rhs)
{

}

// MARK: - Evaluation

static auto __resolve_variable(const kdl::lexeme var, std::map<std::string, kdl::lexeme> table) -> kdl::lexeme
{
    if (!var.is(kdl::lexeme::var)) {
        return var;
    }

    if (table.find(var.text()) == table.end()) {
        kdl::log::fatal_error(var, 1, "Unknown variable encountered in assertion '" + var.text() + "'");
    }

    return table.at(var.text());
}

auto kdl::assertion::evaluate(std::map<std::string, lexeme> variables) const -> bool
{
    auto lhs = __resolve_variable(m_lhs, variables);
    auto rhs = __resolve_variable(m_rhs, variables);

    if (lhs.type() != rhs.type()) {
        kdl::log::fatal_error(lhs, 1, "Type mismatch in assertion. Both LHS and RHS must be of the same type.");
    }

    switch (lhs.type()) {
        case kdl::lexeme::integer:
        case kdl::lexeme::res_id:
        case kdl::lexeme::percentage: {
#if (_WIN64 || _WIN32) && (_MSC_VER)
            auto v1 = lhs.value<int64_t>();
            auto v2 = rhs.value<int64_t>();
#else
            auto v1 = lhs.value<__int128>();
            auto v2 = rhs.value<__int128>();
#endif

            switch (m_operation) {
                case lt:    return v1 < v2;
                case lteq:  return v1 <= v2;
                case eq:    return v1 == v2;
                case neq:   return v1 != v2;
                case gteq:  return v1 >= v2;
                case gt:    return v1 > v2;
            }
        };

        case kdl::lexeme::string: {
            auto v1 = lhs.text();
            auto v2 = rhs.text();
            switch (m_operation) {
                case eq:    return v1 == v2;
                case neq:   return v1 != v2;
                default: {
                    kdl::log::fatal_error(lhs, 1, "Operator not supported for string types.");
                }
            }
        };

        default: {
            kdl::log::fatal_error(lhs, 1, "Unsupported type found in assertion.");
        }
    }
}

// MARK: - Reasons

auto kdl::assertion::failure_text() const -> std::string
{
    std::string reason;
    reason.append(m_lhs.text());

    switch (m_operation) {
        case lt:    reason.append(" must be less than "); break;
        case lteq:  reason.append(" must be less than or equal to "); break;
        case eq:    reason.append(" must be equal to "); break;
        case neq:   reason.append(" must not be equal to "); break;
        case gteq:  reason.append(" must be greater than or equal to "); break;
        case gt:    reason.append(" must be greater than "); break;
    }

    reason.append(m_rhs.text());
    return reason;
}