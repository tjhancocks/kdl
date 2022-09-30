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
#include <libKDL/lexer/lexeme.hpp>
#include <libKDL/host/filesystem/file.hpp>

using namespace kdl;

// MARK: - Construction Tests

TEST(lexeme_construct_usingTextAndType)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::equal(lx.text(), std::string("lexeme"));
    test::equal(lx.type(), lexer::lexeme::string);
}

TEST(lexeme_construct_usingText_fullInfo)
{
    auto file = std::make_shared<host::filesystem::file>("/path/to/TestFile.kdl", "Contents");
    lexer::lexeme lx("lexeme", lexer::lexeme::string, 1, 2, 3, file);
    test::equal(lx.type(), lexer::lexeme::string);
    test::equal(lx.text(), std::string("lexeme"));
    test::equal(lx.offset(), 2);
    test::equal(lx.line(), 3);
    test::equal(lx.location(), std::string("/path/to/TestFile.kdl:L3:2"));
}

TEST(lexeme_construct_resourceReferenceUsingComponents)
{
    lexer::lexeme lx({ "Namespace", "Type", "128" }, lexer::lexeme::res_id, 1, 2, 3, {});
    test::equal(lx.text(), std::string("Namespace.Type.128"));
    test::equal(lx.components().size(), 3);
}

// MARK: - Components / Text

TEST(lexeme_components_hasSingleValueMatchingText_whenNoComponents)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::equal(lx.components().front(), lx.text());
}

TEST(lexeme_components_hasExpectedValue_whenSingleComponent)
{
    lexer::lexeme lx(std::vector<std::string>(1, "Namespace"), lexer::lexeme::res_id, 1, 2, 3, {});
    test::equal(lx.components().size(), 1);
    test::equal(lx.components().front(), std::string("Namespace"));
}

TEST(lexeme_components_hasExpectedValue_whenMultipleComponents)
{
    lexer::lexeme lx({ "Namespace", "Type", "128" }, lexer::lexeme::res_id, 1, 2, 3, {});
    test::equal(lx.components().size(), 3);
    test::equal(lx.components().at(0), std::string("Namespace"));
    test::equal(lx.components().at(1), std::string("Type"));
    test::equal(lx.components().at(2), std::string("128"));
}

TEST(lexeme_text_hasExpectedValue)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::equal(lx.text(), std::string("lexeme"));
}

TEST(lexeme_text_matchesValueOfCombinedComponents_whenMultipleComponents)
{
    lexer::lexeme lx({ "Namespace", "Type", "128" }, lexer::lexeme::res_id, 1, 2, 3, {});
    test::equal(lx.text(), std::string("Namespace.Type.128"));
}

// MARK: - Equality / 'is'

TEST(lexeme_isType_matchesCorrectly)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::is_true(lx.is(lexer::lexeme::string));
}

TEST(lexeme_isType_doesNotMatchCorrectly)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::is_false(lx.is(lexer::lexeme::integer));
}

TEST(lexeme_isText_matchesCorrectly)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::is_true(lx.is("lexeme"));
}

TEST(lexeme_isText_doesNotMatchCorrectly)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::is_false(lx.is("incorrect"));
}

TEST(lexeme_isTypeAndText_matchesCorrectly)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::is_true(lx.is(lexer::lexeme::string, "lexeme"));
}

TEST(lexeme_isTypeAndText_doesNotMatchTypeCorrectly)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::is_false(lx.is(lexer::lexeme::integer, "lexeme"));
}

TEST(lexeme_isTypeAndText_doesNotMatchTextCorrectly)
{
    lexer::lexeme lx("lexeme", lexer::lexeme::string);
    test::is_false(lx.is(lexer::lexeme::string, "incorrect"));
}