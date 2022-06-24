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

#pragma once

#include <vector>
#include <tuple>
#include <string>
#include "target/new/binary_type.hpp"
#include "parser/lexeme.hpp"

namespace kdl::build_target
{

    /**
     * The type template is a structure that defines what the binary layout and structure of a resource
     * look like. It is resembles and is based upon the 'TMPL' resource from the Classic Macintosh.
     */
    class type_template
    {
    public:

        /**
         * The binary_field structure represents the label and type of a field in a binary template:
         *
         *      DWRD FieldLabel;
         *
         */
        struct binary_field
        {
        public:
            lexeme label;
            binary_type type;

            binary_field(const lexeme& label, const binary_type& type);
        };

    public:
        type_template() = default;

        auto add_binary_field(const binary_field& field) -> void;

        [[nodiscard]] auto binary_field_count() const -> std::size_t;
        [[nodiscard]] auto binary_field_at(const std::size_t& index) const -> binary_field;
        [[nodiscard]] auto binary_field_named(const std::string& name) const -> binary_field;
        [[nodiscard]] auto binary_field_named(const lexeme& lx) const -> binary_field;
        [[nodiscard]] auto binary_field_index(const std::string& name) const -> int;
        [[nodiscard]] auto binary_field_index(const lexeme& lx) const -> int;

    private:
        std::vector<binary_field> m_fields;

    };

}
