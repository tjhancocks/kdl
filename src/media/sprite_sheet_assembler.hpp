// Copyright (c) 2020 Tom Hancocks
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

#if !defined(KDL_SPRITE_SHEET_ASSEMBLER_HPP)
#define KDL_SPRITE_SHEET_ASSEMBLER_HPP

#include <vector>
#include "parser/lexeme.hpp"

namespace kdl { namespace media {

    class sprite_sheet_assembler
    {
    private:
        std::vector<std::shared_ptr<std::vector<char>>> m_input_file_contents;
        lexeme m_input_file_format;

    public:
        sprite_sheet_assembler(const std::vector<std::vector<char>> input_file_contents, const lexeme input);

        auto assemble() const -> std::vector<char>;
    };

}}

#endif //KDL_SPRITE_SHEET_ASSEMBLER_HPP
