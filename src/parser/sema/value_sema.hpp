//
// Created by Tom Hancocks on 05/04/2020.
//

#if !defined(KDL_VALUE_SEMA_HPP)
#define KDL_VALUE_SEMA_HPP

#include "parser/parser.hpp"
#include "target/target.hpp"
#include "target/resource.hpp"

namespace kdl { namespace sema {

    struct value_sema
    {
    public:

        static auto parse_any_reference(parser& parser, const field field, const field_value value, const type type,
                                        resource& resource_data) -> void;

        static auto parse_named_type(parser& parser, const lexeme name, const field field, const field_value value,
                                     const type type, std::weak_ptr<target> target, resource& resource_data) -> void;

        static auto parse_file_type(parser& parser, const field field, const field_value value, const type type,
                                    std::weak_ptr<target> target, resource& resource_data) -> void;

        static auto parse_bitmask_type(parser& parser, const field field, const field_value value, const type type,
                                       resource& resource_data) -> void;

        static auto parse_value(parser& parser, const field field, const field_value value, const type type,
                                resource& resource_data) -> void;

    };

}};

#endif //KDL_VALUE_SEMA_HPP
