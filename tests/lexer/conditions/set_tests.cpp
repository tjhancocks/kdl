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
#include <libKDL/lexer/conditions/set.hpp>

using namespace kdl::lexer::condition;

TEST(lexerTemplateConditional_setContains_trueIfStringConsistsOnlyOfCharactersInSet)
{
    test::is_true(set<'A', 'Z'>::contains("AAAZZAAA"));
}

TEST(lexerTemplateConditional_setContains_falseIfStringHasCharactersNotInSet)
{
    test::is_false(set<'A', 'Z'>::contains("AAA ZZ AAA"));
}

TEST(lexerTemplateConditional_setNotContains_trueIfStringHasCharactersNotInSet)
{
    test::is_true(set<'A', 'Z'>::not_contains("AAA ZZ AAA"));
}

TEST(lexerTemplateConditional_setNotContains_falseIfStringConsistsOnlyOfCharactersInSet)
{
    test::is_false(set<'A', 'Z'>::not_contains("AAAZZAAA"));
}