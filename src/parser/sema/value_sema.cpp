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
#include "parser/sema/value_sema.hpp"
#include "parser/file.hpp"
#include "target/resource.hpp"

// MARK: - Explicit Type: `as &`

auto kdl::sema::value_sema::parse_any_reference(kdl::sema::parser &parser, const kdl::field field,
                                                const kdl::field_value value, const kdl::type type,
                                                kdl::resource& resource_data) -> void
{
    if (!parser.expect({ expectation(lexeme::res_id).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "The field '" + field.name() + "' expects a resource id.");
    }
    auto ref = parser.read();

    // Ensure the type is valid for a reference.
    switch (type & 0xF000) {
        case kdl::DWRD: {
            resource_data.write_signed_short(value, ref.value<int16_t>());
            break;
        }
        case kdl::DQAD: {
            resource_data.write_signed_quad(value, ref.value<int64_t>());
            break;
        }
        default: {
            log::fatal_error(field.name_lexeme(), 1, "Resource reference value should be backed by either a DWRD or DQAD value.");
        }
    }
}

// MARK: - Explicit Type: `as NamedType`

auto kdl::sema::value_sema::parse_named_type(kdl::sema::parser &parser, const field_value_type value_type,
                                             const kdl::field field, const kdl::field_value value, const kdl::type type,
                                             std::weak_ptr<kdl::target> target, kdl::resource &resource_data) -> void
{
    if (value_type.name() == "File") {
        parse_file_type(parser, field, value, type, target, resource_data);
    }
    else if (value_type.name() == "Bitmask") {
        parse_bitmask_type(parser, field, value, type, resource_data);
    }
    else if (value_type.name() == "Range") {
        parse_range_type(parser, value_type, field, value, type, resource_data);
    }
    else {
        log::fatal_error(value_type.name_lexeme().value(), 1, "Unrecognised type name '" + value_type.name() + "'");
    }
}

// MARK: - Explicit Type: `as File`

auto kdl::sema::value_sema::parse_file_type(kdl::sema::parser &parser, const kdl::field field,
                                            const kdl::field_value value, const kdl::type type,
                                            std::weak_ptr<kdl::target> target, kdl::resource &resource_data) -> void
{
    auto t = target.lock();

    // The field type is the builtin File type. The value should be a string representing a file
    // path. The contents of the file should be loaded into the field as the value.
    auto import_file = false;
    if (parser.expect({ expectation(lexeme::identifier, "import").be_true() })) {
        parser.advance();
        import_file = true;
    }

    if (!parser.expect({ expectation(lexeme::string).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Fields with the 'File' type expect a string value.");
    }
    auto string_lx = parser.read();
    auto string_value = string_lx.text();

    if (import_file) {
        auto path = t->resolve_src_path(string_value);
        string_value = kdl::file(path).contents();
    }

    // Get the value type for the field
    switch (type & 0xF000) {
        case kdl::PSTR: {
            if (string_value.size() > 255) {
                log::fatal_error(string_lx, 1, "String too large for value type.");
            }
            resource_data.write_pstr(value, string_value);
            break;
        }
        case kdl::CSTR: {
            resource_data.write_cstr(value, string_value);
            break;
        }
        case kdl::HEXD: {
            // Use a cstr write function with a set size to exclude the terminating NUL byte.
            resource_data.write_data(value, string_value);
            break;
        }
        case kdl::Cxxx: {
            auto size = type_size(type);
            if (string_value.size() > size) {
                log::fatal_error(string_lx, 1, "String too large for value type.");
            }
            resource_data.write_cstr(value, string_value, size);
            break;
        }
        default: {
            log::fatal_error(string_lx, 1, "Unsupported value type for field '" + field.name() + "' with a type 'File'.");
        }
    }
}

// MARK: - Explicit Type: `as Bitmask`

auto kdl::sema::value_sema::parse_bitmask_type(kdl::sema::parser &parser, const kdl::field field,
                                               const kdl::field_value value, const kdl::type type,
                                               kdl::resource &resource_data) -> void
{
    if (field.value_count() != 1) {
        log::fatal_error(parser.peek(-1), 1, "The field '" + field.name() + "' should have only one value due to it being a 'Bitmask'.");
    }

    if (type != kdl::HBYT && type != kdl::HWRD && type != kdl::HLNG && type != kdl::HQAD) {
        log::fatal_error(parser.peek(-1), 1, "The field '" + field.name() + "' must be backed by either a HBYT, HWRD, HLNG or HQAD value.");
    }

    uint64_t mask = 0;

    while (parser.expect({ expectation(lexeme::semi).be_false() })) {
        if (parser.expect({ expectation(lexeme::integer).be_true() })) {
            // Merge in an integer.
            mask |= parser.read().value<uint64_t>();
        }
        else if (parser.expect({ expectation(lexeme::identifier).be_true() })) {
            // Look up symbol and merge it.
            auto symbol = parser.read();
            auto symbol_value = value.value_for(symbol);

            if (!symbol_value.is(lexeme::integer)) {
                log::fatal_error(symbol, 1, "Type mismatch for '" + symbol.text() + "' in bitmask.");
            }

            mask |= symbol_value.value<uint64_t>();
        }
        else {
            auto lx = parser.peek();
            log::fatal_error(lx, 1, "Unexpected lexeme encountered in bitmask: '" + lx.text() + "'");
        }

        if (!parser.expect({ expectation(lexeme::semi).be_true() })) {
            parser.ensure({ expectation(lexeme::pipe).be_true() });
        }
    }

    switch (type & 0xF000) {
        case kdl::HBYT: {
            resource_data.write_byte(value, static_cast<uint8_t>(mask & 0xFF));
            break;
        }
        case kdl::HWRD: {
            resource_data.write_short(value, static_cast<uint16_t>(mask & 0xFFFF));
            break;
        }
        case kdl::HLNG: {
            resource_data.write_long(value, static_cast<uint32_t>(mask & 0xFFFFFFFF));
            break;
        }
        case kdl::HQAD: {
            resource_data.write_quad(value, mask);
            break;
        }
        default: {
            throw std::logic_error("Unexpected Bitmask type encountered.");
        }
    }
}

// MARK: - Explicit Type: `as Range<#, #>

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
static auto __validate_range(const kdl::lexeme value, const kdl::lexeme lower, const kdl::lexeme upper) -> T
{
    auto __v = value.value<T>();
    auto __lV = lower.value<T>();
    auto __uV = upper.value<T>();
    if ((__v >= __lV) && (__v <= __uV)) {
        return __v;
    }
    else {
        kdl::log::fatal_error(value, 1, "Specified value '" + value.text() + "' is outside of allowed range '" + lower.text() + " ... " + upper.text() + "'");
    }
}

auto kdl::sema::value_sema::parse_range_type(kdl::sema::parser &parser,  const field_value_type value_type,
                                             const kdl::field field, const kdl::field_value value, const kdl::type type,
                                             kdl::resource &resource_data) -> void
{
    // A range requires two associated numeric values.
    if (value_type.associated_count() != 2) {
        log::fatal_error(value_type.name_lexeme().value(), 1, "The 'Range' type requires an upper and a lower bound to be provided.");
    }

    auto lower = value_type.associated_at(0);
    auto upper = value_type.associated_at(1);

    if (!(lower.is(lexeme::integer) || lower.is(lexeme::percentage) || lower.is(lexeme::res_id))) {
        log::fatal_error(lower, 1, "Lower bound of 'Range' type must be a numeric value.");
    }
    if (!(upper.is(lexeme::integer) || upper.is(lexeme::percentage) || upper.is(lexeme::res_id))) {
        log::fatal_error(lower, 1, "Upper bound of 'Range' type must be a numeric value.");
    }
    if (lower.type() != upper.type()) {
        log::fatal_error(lower, 1, "Lower and Upper bound of 'Range' type must be of the same type.");
    }

    // Get the value - we need to know the backing store so that we know what type of integer to work with.
    if (!parser.expect({ expectation(lower.type()).be_true() })) {
        auto lx = parser.peek();
        log::fatal_error(lx, 1, "Incorrect value type provided for field '" + field.name() + "'");
    }

    switch (type & 0xF000) {
        case kdl::DBYT: {
            resource_data.write_signed_byte(value, __validate_range<int8_t>(parser.read(), lower, upper));
            break;
        }
        case kdl::DWRD: {
            resource_data.write_signed_short(value, __validate_range<int16_t>(parser.read(), lower, upper));
            break;
        }
        case kdl::DLNG: {
            resource_data.write_signed_long(value, __validate_range<int32_t>(parser.read(), lower, upper));
            break;
        }
        case kdl::DQAD: {
            resource_data.write_signed_quad(value, __validate_range<int64_t>(parser.read(), lower, upper));
            break;
        }
        case kdl::HBYT: {
            resource_data.write_byte(value, __validate_range<uint8_t>(parser.read(), lower, upper));
            break;
        }
        case kdl::HWRD: {
            resource_data.write_short(value, __validate_range<uint16_t>(parser.read(), lower, upper));
            break;
        }
        case kdl::HLNG: {
            resource_data.write_long(value, __validate_range<uint32_t>(parser.read(), lower, upper));
            break;
        }
        case kdl::HQAD: {
            resource_data.write_quad(value, __validate_range<uint64_t>(parser.read(), lower, upper));
            break;
        }
        default: {
            throw std::logic_error("Attempting to handle 'Range' type with a none numeric backing type.");
        }
    }
}


// MARK: - Inferred Value

auto kdl::sema::value_sema::parse_value(kdl::sema::parser &parser, const kdl::field field, const kdl::field_value value,
                                        const kdl::type type, kdl::resource &resource_data) -> void
{
    // Read the next value and write it to the resource.
    switch (type & 0xF000) {
        case kdl::DBYT: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_signed_byte(value, parser.read().value<int8_t>());
            break;
        }
        case kdl::DWRD: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_signed_short(value, parser.read().value<int16_t>());
            break;
        }
        case kdl::DLNG: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_signed_long(value, parser.read().value<int32_t>());
            break;
        }
        case kdl::DQAD: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_signed_quad(value, parser.read().value<int64_t>());
            break;
        }
        case kdl::HBYT: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_byte(value, parser.read().value<uint8_t>());
            break;
        }
        case kdl::HWRD: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_short(value, parser.read().value<uint16_t>());
            break;
        }
        case kdl::HLNG: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_long(value, parser.read().value<uint32_t>());
            break;
        }
        case kdl::HQAD: {
            if (!parser.expect({ expectation(lexeme::integer).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an integer literal for field '" + field.name() + "'.");
            }
            resource_data.write_quad(value, parser.read().value<uint64_t>());
            break;
        }

        case kdl::PSTR: {
            if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an string literal for field '" + field.name() + "'.");
            }
            resource_data.write_pstr(value, parser.read().text());
            break;
        }
        case kdl::CSTR: {
            if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an string literal for field '" + field.name() + "'.");
            }
            resource_data.write_cstr(value, parser.read().text());
            break;
        }

        case kdl::Cxxx: {
            if (!parser.expect({ expectation(lexeme::string).be_true() })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected an string literal for field '" + field.name() + "'.");
            }
            resource_data.write_cstr(value, parser.read().text(), type_size(type));
            break;
        }

        case kdl::RECT: {
            if (!parser.expect({
                expectation(lexeme::integer).be_true(),
                expectation(lexeme::integer).be_true(),
                expectation(lexeme::integer).be_true(),
                expectation(lexeme::integer).be_true()
            })) {
                auto lx = parser.peek();
                log::fatal_error(lx, 1, "Expected 4 integer literals for field '" + field.name() + "'.");
            }
            resource_data.write_rect(value,
                                     parser.read().value<int16_t>(),
                                     parser.read().value<int16_t>(),
                                     parser.read().value<int16_t>(),
                                     parser.read().value<int16_t>());
            break;
        }

        case kdl::HEXD: {
            log::fatal_error(parser.peek(), 1, "The 'HEXD' type is not directly supported '" + field.name() + "'.");
        }

        case NONE: {
            log::fatal_error(parser.peek(), 1, "Unknown type encountered in field '" + field.name() + "'.");
        }
    }
}
