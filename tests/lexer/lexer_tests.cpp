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

using namespace kdl::lexer;

// MARK: - Correct Lexeme Parsing

TEST(lexer_analyze_returnsExpectedSequenceOfLexemes_noErrors)
{
    std::string source = R"(@out "Hello, World!";)";
    auto file = std::make_shared<kdl::host::filesystem::file>("TestFile.kdl", source);

    test::does_not_throw([&] {
        test::measure([&] {
            auto result = lexer(file).analyze();
            test::equal(result.size(), 3);
            test::is_true(result.at(0).is(lexeme::directive, "out"));
            test::is_true(result.at(1).is(lexeme::string, "Hello, World!"));
            test::is_true(result.at(2).is(lexeme::semi));
        }, "Perform Lexical Analysis");
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

TEST(lexer_analyze_producesExpectedLexemesForInputs)
{
    std::vector<std::tuple<std::string, enum lexeme::type, std::string>> lexeme_expectations({
        { "@out", lexeme::directive, "out" },
        { "\"string literal\"", lexeme::string, "string literal" },
        { "#auto", lexeme::res_id, "auto" },
        { "#128", lexeme::res_id, "128" },
        { "#foo.128", lexeme::res_id, "foo.128" },
        { "#foo.bar.128", lexeme::res_id, "foo.bar.128" },
        { "#-128", lexeme::res_id, "-128" },
        { "$(", lexeme::l_expr, "" },
//        { ")", lexeme::r_expr, "" },  NOTE: Currently no way to test this one due to being stateful.
        { "$var", lexeme::var, "var" },
        { "0xFEED", lexeme::integer, "0xFEED" },
        { "0XBEEF", lexeme::integer, "0xBEEF" },
        { "123", lexeme::integer, "123" },
        { "-103", lexeme::integer, "-103" },
        { "-54%", lexeme::percentage, "-54" },
        { "96%", lexeme::percentage, "96" },
        { "_identifier", lexeme::identifier, "_identifier" },
        { "Identifier", lexeme::identifier, "Identifier" },
        { "Identifier2", lexeme::identifier, "Identifier2" },
        { ";", lexeme::semi, ";" },
        { "{", lexeme::l_brace, "{" },
        { "}", lexeme::r_brace, "}" },
        { "[", lexeme::l_bracket, "[" },
        { "]", lexeme::r_bracket, "]" },
        { "(", lexeme::l_paren, "(" },
        { ")", lexeme::r_paren, ")" },
        { "<", lexeme::l_angle, "<" },
        { ">", lexeme::r_angle, ">" },
        { "=", lexeme::equals, "=" },
        { "+", lexeme::plus, "+" },
        { "-", lexeme::minus, "-" },
        { "*", lexeme::star, "*" },
        { "/", lexeme::slash, "/" },
        { "&", lexeme::amp, "&" },
        { ".", lexeme::dot, "." },
        { ",", lexeme::comma, "," },
        { "|", lexeme::pipe, "|" },
        { "^", lexeme::carat, "^" },
        { ":", lexeme::colon, ":" },
        { "!", lexeme::exclaim, "!" },
        { "<<", lexeme::left_shift, "<<" },
        { ">>", lexeme::right_shift, ">>" }
    });

    for (const auto& expectation : lexeme_expectations) {
        auto file = std::make_shared<kdl::host::filesystem::file>("TestFile.kdl", std::get<0>(expectation));
        auto result = lexer(file).analyze();
        test::equal(result.size(), 1, "Incorrect number of lexemes produced for: " + std::get<0>(expectation));
        test::is_true(result.at(0).is(std::get<1>(expectation), std::get<2>(expectation)), "Failed to correctly lexically analyze: " + std::get<0>(expectation));
    }
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