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

#if !defined(KDL_LEXEME_HPP)
#define KDL_LEXEME_HPP

#include <memory>
#include <string>
#include <type_traits>
#include "parser/file.hpp"

namespace kdl
{
    struct file;

    /**
     * The kdl::lexeme structure represents a single token/lexeme within a KDL source file. It has
     * the source text value, the position of it within the file, and the owning file from which it
     * came.
     */
    struct lexeme
    {
    public:
        enum type
        {
            any,
            integer, string, res_id, identifier,
            l_paren, r_paren, l_angle, r_angle, l_brace, r_brace, l_bracket, r_bracket,
            comma, dot, pipe, slash, amp, colon, semi, plus, minus, star, equals, carat,
            directive, var, percentage
        };

    private:
        std::weak_ptr<file> m_owner;
        std::string m_text;
        std::size_t m_pos;
        std::size_t m_offset;
        std::size_t m_line;
        type m_type;

    public:
        /**
         * Constructs a new lexeme.
         * @param text The source text value from which this lexeme is being created.
         * @param type The lexical type that the token is
         * @param pos The absolute position of the token within the source file.
         * @param offset The position of the token upon the current line.
         * @param line The line that the token was found.
         * @param owner The file from which the token originated.
         */
        lexeme(std::string text, enum type type, std::size_t pos, std::size_t offset, std::size_t line, std::weak_ptr<file> owner)
            : m_text(text), m_type(type), m_pos(pos), m_offset(offset), m_line(line), m_owner(owner)
        {

        }

        /**
         * Returns a string describing the location of the lexeme, for example:
         *
         * path/to/file.kdl:L30:10
         *
         * @return A string representing the location of the lexeme.
         */
        auto location() const -> std::string
        {
            std::string result;

            // Only attach the file if we still have a valid reference to it. If the file has been
            // released then omit it.
            if (auto file = m_owner.lock()) {
                result += file->path() + ":";
            }

            // Encode the line and offset, and return the result.
            result += "L" + std::to_string(m_line) + ":" + std::to_string(m_offset);
            return result;
        }

        /**
         * Check if the lexeme is of a specific type.
         * @param type A lexeme type.
         * @return true if matching
         */
        auto is(const enum type type) const -> bool
        {
            return type == m_type;
        }

        /**
         * Check if the lexeme has the specified value.
         * @param value The value to test for.
         * @return true if matching
         */
        auto is(const std::string value) const -> bool
        {
            return value == m_text;
        }

        /**
         * Check if the lexeme has the specified type and value.
         * @param type A lexeme type.
         * @param value The value to test for.
         * @return true if matching
         */
        auto is(const enum type type, const std::string value) const -> bool
        {
            return is(type) && is(value);
        }

        /**
         * The line number of the lexeme.
         * @return An integer representing the line number.
         */
        auto line() const -> std::size_t
        {
            return m_line;
        }

        /**
         * The line offset of the lexeme.
         * @return An integer representing the line offset.
         */
        auto offset() const -> std::size_t
        {
            return m_offset;
        }

        /**
         * The lexical type of the lexeme.
         * @return A lexeme type.
         */
        auto type() const -> enum type
        {
            return m_type;
        }

        /**
         * The textual value of the lexeme
         * @return A string
         */
        auto text() const -> std::string
        {
            return m_text;
        }

        /**
         * The numeric value of the lexeme. If the lexeme is not a numeric type then the value returned will
         * be 0.
         */
        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto value() const -> T
        {
           if (m_text.find_first_not_of("0123456789ABCDEFabcdef") == std::string::npos) {
               // Hex
               return static_cast<T>(std::stoull(m_text, nullptr, 16));
           }
           else {
               // Decimal
               return static_cast<T>(std::stoull(m_text, nullptr, 10));
           }
        }

    };

};

#endif //KDL_LEXEME_HPP
