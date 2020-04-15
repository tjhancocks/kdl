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

#if !defined(KDL_BUILD_TARGET_TYPE_CONTAINER_HPP)
#define KDL_BUILD_TARGET_TYPE_CONTAINER_HPP

#include <any>
#include <string>
#include <memory>
#include <optional>
#include "target/new/type_template.hpp"
#include "target/new/type_field.hpp"
#include "parser/lexeme.hpp"
#include "target/new/resource_instance.hpp"
#include "target/assertion.hpp"

namespace kdl { namespace build_target {

    /**
     * The type_container represents a resource type. It contains all of the meta data/information
     * and structural information needed by KDL to assemble resources.
     */
    class type_container
    {
    private:
        std::string m_code;
        std::string m_name;
        type_template m_tmpl;
        std::vector<type_field> m_fields;
        std::vector<assertion> m_assertions;

    public:
        type_container(const std::string code);
        type_container(const lexeme name, const std::string code);
        type_container(const std::string name, const std::string code);

        auto name() const -> std::string;
        auto code() const -> std::string;

        auto internal_template() -> type_template&;
        auto set_internal_template(const type_template tmpl) -> void;

        auto add_field(const lexeme name) -> type_field&;
        auto add_field(const type_field field) -> void;
        auto field_named(const lexeme name) -> type_field;
        auto all_fields() const -> std::vector<type_field>;

        auto assertions() const -> std::vector<assertion>;
        auto add_assertion(const assertion assertion) -> void;
        auto add_assertions(const std::vector<assertion> assertions) -> void;

        auto new_instance(const int64_t id, std::optional<std::string> name = {}) -> resource_instance;
    };

}};

#endif //KDL_TYPE_CONTAINER_HPP
