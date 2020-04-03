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

#if !defined(KDL_SEMA_HPP)
#define KDL_SEMA_HPP

#include <memory>
#include <vector>
#include "parser/lexeme.hpp"
#include "target/target.hpp"
#include "parser/expectation.hpp"

namespace kdl { namespace sema {

    /**
     * The kdl::sema::parser class is responsible for consuming a stream of lexemes (std::vector<lexeme>)
     * and interpreting what is seen.
     */
    class parser
    {
    private:
        std::weak_ptr<target> m_target;
        std::size_t m_ptr { 0 };
        const std::vector<lexeme> m_lexemes;

        /**
         * Check if the parser has reached the end of the token stream.
         * @param offset The offset from the current position in the token stream.
         * @param count The required number of lexemes to be considered.
         * @return true if all required lexemes are present.
         */
        auto finished(const long offset = 0, const long count = 1) const -> bool;

        /**
         * Keep consuming lexemes from the stream, until the expectation is no longer being met.
         * @param expect The expectation function for matching lexemes.
         * @return A list of all lexemes that were matched.
         */
        auto consume(expectation::function expect) -> std::vector<lexeme>;

        /**
         * Advance the lexeme stream by the specified amount.
         * @param delta The number of lexemes to advance by.
         */
        auto advance(const long delta = 1) -> void;

        /**
         * Peek a lexeme from the lexeme stream without advancing the current position.
         * @param offset The offset from the current position in the token stream.
         * @return A lexeme.
         */
        auto peek(const long offset = 0) const -> lexeme;

        /**
         * Read a lexeme from the lexeme stream, and advance the position to the next token after the one that was
         * read.
         * @param offset The offset from the current position in the token stream.
         * @return A lexeme.
         */
        auto read(const long offset = 0) -> lexeme;

        /**
         * Validate a sequence of lexemes with the specified list of expectations. There is a one to one mapping
         * between expectations and lexemes.
         * @param expect A list of expectations.
         * @return true if all lexemes met their expectations.
         */
        auto expect(std::initializer_list<expectation::function> expect) const -> bool;

        /**
         * Validate a sequence of lexemes with the specified list of expectations. There is a one to one mapping
         * between expectations and lexemes. Each lexeme that meets its expectation is advanced past.
         *
         * If any of the expectations are not met, a fatal error is raised.
         * @param expect A list of expectations.
         */
        auto ensure(std::initializer_list<expectation::function> expect) -> void;

        /**
         * Insert new lexemes into the parser at the current location.
         * @param lexemes The list of lexemes to be inserted.
         */
        auto insert(std::vector<lexeme> lexemes) -> void;


    public:
        /**
         * Construct a new parser for the specified target and with the provided lexeme stream.
         * @param target The destination target of the parser.
         * @param lexemes The lexemes to be parsed.
         */
        parser(std::weak_ptr<target> target, std::vector<lexeme> lexemes);

        /**
         * Parse the lexeme stream into/against the target.
         */
        auto parse() -> void;


    };

}};

#endif //KDL_SEMA_HPP