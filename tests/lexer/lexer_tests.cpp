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

#include <libTesting/testing.hpp>
#include <libKDL/lexer/lexer.hpp>
#include <libKDL/exception/unrecognised_character_exception.hpp>
#include <libKDL/exception/lexical_exception.hpp>

using namespace kdl::lexer;

// MARK: - Correct Lexeme Parsing

TEST(lexer_analyze_returnsExpectedSequenceOfLexemes_noErrors)
{
    std::string source = R"(@out "Hello, World!";)";
    auto file = std::make_shared<kdl::host::filesystem::file>("TestFile.kdl", source);

    test::does_not_throw([&] {
        auto result = lexer(file).analyze();
        test::equal(result.size(), 3);
        test::is_true(result.at(0).is(lexeme::directive, "out"));
        test::is_true(result.at(1).is(lexeme::string, "Hello, World!"));
        test::is_true(result.at(2).is(lexeme::semi));
    });
}

TEST(lexer_analzye_commentsAreIgnored)
{
    std::string source = R"(
    ` @out "Hello, World!";
    )";
    auto file = std::make_shared<kdl::host::filesystem::file>("TestFile.kdl", source);

    test::does_not_throw([&] {
        auto result = lexer(file).analyze();
        test::is_true(result.empty());
    });
}

// MARK: - Exceptions

TEST(lexer_analyze_unrecognisedCharacterExceptionRaised_whenExpected)
{
    std::string source = R"(§)";
    auto file = std::make_shared<kdl::host::filesystem::file>("TestFile.kdl", source);

    test::does_throw<kdl::unrecognised_character_exception>([&] {
        lexer(file).analyze();
    });
}