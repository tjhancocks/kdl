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

#if !defined(KDL_TYPES_HPP)
#define KDL_TYPES_HPP

#include <cstdint>

namespace kdl
{

    enum type : uint16_t
    {
        DBYT = 0x0000,
        DWRD = 0x1000,
        DLNG = 0x2000,
        DQAD = 0x3000,
        HBYT = 0x4000,
        HWRD = 0x5000,
        HLNG = 0x6000,
        HQAD = 0x7000,
        RECT = 0x8000,
        PSTR = 0x9000,
        CSTR = 0xA000,
        Cxxx = 0xB000,
        HEXD = 0xC000
    };

    auto type_size(enum type type) -> std::size_t
    {
        switch (type & 0xF000) {
            case DBYT:
            case HBYT:
                return 1;

            case DWRD:
            case HWRD:
                return 2;

            case DLNG:
            case HLNG:
                return 4;

            case DQAD:
            case HQAD:
            case RECT:
                return 8;

            case PSTR:
            case CSTR:
            case HEXD:
            default:
                // They are variable length.
                return 0;

            case Cxxx:
                return (type & 0x0FFF);
        }
    }

};

#endif //KDL_TYPES_HPP
