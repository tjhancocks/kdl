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
#include <libKDL/lexer/conditions/range.hpp>

using namespace kdl::lexer::condition;

TEST(lexerTemplateConditional_rangeContains_reportsTrueIfCharacterIsInRange)
{
    test::is_true(range<'0', '9'>::contains("5"));
    test::is_true(range<'0', '9'>::contains("0"));
    test::is_true(range<'0', '9'>::contains("9"));
}

TEST(lexerTemplateConditional_rangeNotContains_reportsFalseIfCharacterIsInRange)
{
    test::is_false(range<'0', '9'>::not_contains("5"));
    test::is_false(range<'0', '9'>::not_contains("0"));
    test::is_false(range<'0', '9'>::not_contains("9"));
}

TEST(lexerTemplateConditional_rangeContains_reportsFalseIfCharacterIsNotInRange)
{
    test::is_false(range<'0', '9'>::contains("a"));
    test::is_false(range<'0', '9'>::contains("!"));
    test::is_false(range<'0', '9'>::contains("A"));
}

TEST(lexerTemplateConditional_rangeNotContains_reportsTrueIfCharacterIsNotInRange)
{
    test::is_true(range<'0', '9'>::not_contains("a"));
    test::is_true(range<'0', '9'>::not_contains("!"));
    test::is_true(range<'0', '9'>::not_contains("A"));
}
