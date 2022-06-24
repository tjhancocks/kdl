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

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <functional>
#include "parser/file.hpp"
#include "parser/lexeme.hpp"

namespace kdl
{

    /**
     * The kdl::lexer class represents a lexical analyser, that will perform lexical analysis on a file,
     * and split it into its component lexemes.
     */
    class lexer
    {
    public:
        /**
         * Construct a new lexer with the specified source file.
         * @param source The source file
         */
        lexer(std::shared_ptr<file> source);

        /**
         * Perform lexical analysis on the source file.
         * @return A vector of lexemes that were the result of lexical analysis.
         */
        auto analyze() -> std::vector<lexeme>;

    private:
        std::shared_ptr<file> m_source;
        std::size_t m_line { 1 };
        std::size_t m_offset { 0 };
        std::size_t m_pos { 0 };
        std::string m_slice;
        bool m_in_expr { false };
        int m_expr_paren_balance { 0 };
        std::vector<lexeme> m_lexemes;

        /**
         * Generates a dummy lexeme based on the current lexer position. This is used for
         * diagnostics and reporting errors.
         */
        [[nodiscard]] auto dummy(long offset = 0) const -> lexeme;

        /**
         * Reports the length of the source file.
         * @return The number of characters in the source file.
         */
        [[nodiscard]] auto length() const -> std::size_t;

        /**
         * Advance the position of the lexer by the specified offset.
         * @param offset The number of characters to advance by.
         */
        auto advance(long offset = 1) -> void;

        /**
         * Test if there are any more characters in the source to parse.
         * @param offset The offset from the current position.
         * @param length The number of characters required.
         * @return true if there are enough remaining characters to meet the requirement.
         */
        [[nodiscard]] auto available(long offset = 0, std::size_t length = 1) const -> bool;

        /**
         * Peek a string from the source without advancing the current position.
         * @param offset The offset from the current position.
         * @param length The number of characters required.
         * @return A string.
         */
        [[nodiscard]] auto peek(long offset = 0, std::size_t length = 1) const -> std::string;

        /**
         * Read a string from the source advancing the current position, to the end of the read string.
         * @param offset The offset from the current position.
         * @param length The number of characters required.
         * @return A string.
         */
        auto read(long offset = 0, std::size_t length = 1) -> std::string;

        /**
         * Test if the specified string from the source, is validated by the provided test function.
         * @param fn Test function to validate the peek'd string from the source.
         * @param offset The offset from the current position.
         * @param length The number of characters required.
         * @return true if the string was validated.
         */
        auto test_if(const std::function<auto(const std::string&) -> bool>& fn, long offset = 0, std::size_t length = 1) const -> bool;

        /**
         * Consume characters from the source, whilst those characters are validated by the test function.
         * @param fn Test function to validate each character.
         * @return true if any characters were matched.
         */
        auto consume_while(const std::function<auto(const std::string&) -> bool>& fn, std::size_t size = 1) -> bool;
    };

    template<char c>
    struct match
    {
        static auto yes(const std::string&) -> bool;
        static auto no(const std::string&) -> bool;
    };

    template<char tc,char...ttC>
    struct sequence
    {
        static auto yes(const std::string&) -> bool;
        static auto no(const std::string&) -> bool;
    };

    template<char lC, char uC>
    struct range
    {
        static auto contains(const std::string&) -> bool;
        static auto not_contains(const std::string&) -> bool;
    };

    template<char tC, char... ttC>
    struct set
    {
        static auto contains(const std::string&) -> bool;
        static auto not_contains(const std::string&) -> bool;
    };

    struct identifier_set
    {
        static auto limited_contains(const std::string&) -> bool;
        static auto contains(const std::string&) -> bool;
    };

    struct decimal_set
    {
        static auto contains(const std::string&) -> bool;
    };

    struct hexadecimal_set
    {
        static auto contains(const std::string&) -> bool;
    };

};
