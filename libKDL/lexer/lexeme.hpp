// Copyright (c) 2019-2022 Tom Hancocks
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

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <libKDL/host/filesystem/file.hpp>

namespace kdl::lexer
{
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
            left_shift, right_shift, tilde,
            directive, var, l_expr, r_expr, percentage, exclaim
        };

    public:
        /**
         * Constructs a new lexeme.
         * @param text The source text value from which this lexeme is being created.
         * @param type The lexical type that the token is
         */
        lexeme(const std::string& text, enum type type)
            : m_text(text), m_type(type), m_owner({}), m_pos(0), m_offset(0), m_line(0)
        {
        }

        /**
         * Constructs a new lexeme.
         * @param text The source text value from which this lexeme is being created.
         * @param type The lexical type that the token is
         * @param pos The absolute position of the token within the source file.
         * @param offset The position of the token upon the current line.
         * @param line The line that the token was found.
         * @param owner The file from which the token originated.
         */
        lexeme(const std::string& text, enum type type, std::size_t pos, std::size_t offset, std::size_t line, std::weak_ptr<host::filesystem::file> owner)
            : m_text(std::move(text)), m_type(type), m_pos(pos), m_offset(offset), m_line(line), m_owner(owner)
        {
        }

        lexeme(const std::vector<std::string>& components, enum type type, std::size_t pos, std::size_t offset, std::size_t line, std::weak_ptr<host::filesystem::file> owner)
            : m_components(components), m_type(type), m_pos(pos), m_offset(offset), m_line(line), m_owner(owner)
        {
            auto is_first = true;
            for (const auto& component : m_components) {
                m_text += (is_first ? "" : ".") + component;
                is_first = false;
            }
        }

        /**
         * Returns the path to the directory, that contains the file from which the lexeme was extracted.
         */
        [[nodiscard]] auto source_directory() const -> host::filesystem::path
        {
            if (auto file = m_owner.lock()) {
                return file->path().parent();
            }
            return {};
        }

        /**
         * Returns a string describing the location of the lexeme, for example:
         *
         * path/to/file.kdl:L30:10
         *
         * @return A string representing the location of the lexeme.
         */
        [[nodiscard]] auto location() const -> std::string
        {
            std::string result;

            // Only attach the file if we still have a valid reference to it. If the file has been
            // released then omit it.
            if (auto file = m_owner.lock()) {
                result += file->path().string() + ":";
            }

            // Encode the line and offset, and return the result.
            result += "L" + std::to_string(m_line) + ":" + std::to_string(m_offset);
            return result;
        }

        [[nodiscard]] auto is(const lexeme& lx) const -> bool
        {
            return (lx.m_text == m_text) && (lx.m_type == m_type);
        }

        /**
         * Check if the lexeme is of a specific type.
         * @param type A lexeme type.
         * @return true if matching
         */
        [[nodiscard]] auto is(const enum type type) const -> bool
        {
            return type == m_type;
        }

        /**
         * Check if the lexeme has the specified value.
         * @param value The value to test for.
         * @return true if matching
         */
        [[nodiscard]] auto is(const std::string& value) const -> bool
        {
            return value == m_text;
        }

        /**
         * Check if the lexeme has the specified type and value.
         * @param type A lexeme type.
         * @param value The value to test for.
         * @return true if matching
         */
        [[nodiscard]] auto is(const enum type type, const std::string& value) const -> bool
        {
            return is(type) && is(value);
        }

        /**
         * The line number of the lexeme.
         * @return An integer representing the line number.
         */
        [[nodiscard]] auto line() const -> std::size_t
        {
            return m_line;
        }

        /**
         * The line offset of the lexeme.
         * @return An integer representing the line offset.
         */
        [[nodiscard]] auto offset() const -> std::size_t
        {
            return m_offset;
        }

        /**
         * The lexical type of the lexeme.
         * @return A lexeme type.
         */
        [[nodiscard]] auto type() const -> enum type
        {
            return m_type;
        }

        /**
         * The textual value of the lexeme
         * @return A string
         */
        [[nodiscard]] auto text() const -> std::string
        {
            if (m_text.empty() && !m_components.empty()) {
                std::string result;
                for (const auto& component : m_components) {
                    if (!result.empty()) {
                        result.insert(result.end(), '.');
                    }
                    result.insert(result.end(), component.begin(), component.end());
                }
                return result;
            }
            return m_text;
        }

        /**
         * Returns a vector of the components that make up the lexeme text.
         * @return  A vector of lexeme components
         */
        [[nodiscard]] auto components() const -> std::vector<std::string>
        {
            if (m_components.empty()) {
                return std::vector<std::string>(1, m_text);
            }
            return m_components;
        }

        /**
         * The numeric value of the lexeme.
         *
         * If the lexeme is not a numeric type or an operator then the value returned will be 0.
         */
        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto value() const -> T
        {
            if (m_type == lexeme::plus || m_type == lexeme::minus) {
                return 2;
            }
            else if (m_type == lexeme::star || m_type == lexeme::slash) {
                return 3;
            }
            else if (m_type == lexeme::carat) {
                return 4;
            }
            else if (m_type == lexeme::left_shift || m_type == lexeme::right_shift) {
                return 5;
            }
            else if (m_type == lexeme::pipe) {
                return 6;
            }
            else if (m_type == lexeme::amp) {
                return 7;
            }
            else if (m_type == lexeme::res_id && !m_components.empty()) {
                return static_cast<T>(std::stoll(m_components.back(), nullptr, 10));
            }
            else if (m_text.size() >= 2 && m_text[0] == '-') {
               // Negative decimal
               return static_cast<T>(std::stoll(m_text, nullptr, 10));
            }
            else if (m_text.size() >= 3 && (m_text.substr(0, 2) == "0x" || m_text.substr(0, 2) == "0X")) {
               // Hex
               return static_cast<T>(std::stoull(m_text.substr(2), nullptr, 16));
            }
            else {
               // Decimal
               return static_cast<T>(std::stoull(m_text, nullptr, 10));
           }
        }

        /**
         * Associativity of the lexeme if it is an operator.
         * Will return true if the operator is left associative
         */
        [[nodiscard]] auto left_associative() const -> bool
        {
            switch (m_type) {
                case lexeme::carat:
                    return false;

                default:
                    return true;
            }
        }

    private:
        std::weak_ptr<host::filesystem::file> m_owner;
        std::string m_text;
        std::size_t m_pos;
        std::size_t m_offset;
        std::size_t m_line;
        std::vector<std::string> m_components;
        enum type m_type;
    };
};