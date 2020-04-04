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
#include <string>

namespace kdl
{

    enum type : uint16_t
    {
        NONE = 0x0000,
        DBYT = 0x1000,
        DWRD = 0x2000,
        DLNG = 0x3000,
        DQAD = 0x4000,
        HBYT = 0x5000,
        HWRD = 0x6000,
        HLNG = 0x7000,
        HQAD = 0x8000,
        RECT = 0x9000,
        PSTR = 0xA000,
        CSTR = 0xB000,
        Cxxx = 0xC000,
        HEXD = 0xD000
    };

    static auto type_size(enum type type) -> std::size_t
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

    static auto type_for_name(const std::string name) -> kdl::type
    {
        if (name == "DBYT") {
            return DBYT;
        }
        else if (name == "DWRD") {
            return DWRD;
        }
        else if (name == "DLNG") {
            return DLNG;
        }
        else if (name == "DQAD") {
            return DQAD;
        }
        else if (name == "HBYT") {
            return HBYT;
        }
        else if (name == "HWRD") {
            return HWRD;
        }
        else if (name == "HLNG") {
            return HLNG;
        }
        else if (name == "HQAD") {
            return HQAD;
        }
        else if (name == "RECT") {
            return RECT;
        }
        else if (name == "HEXD") {
            return HEXD;
        }
        else if (name == "PSTR") {
            return PSTR;
        }
        else if (name == "CSTR") {
            return CSTR;
        }
        else if (name[0] == 'C') {
            auto width = static_cast<uint16_t>(std::stoul(name.substr(1), nullptr, 16));
            return static_cast<kdl::type>(static_cast<uint16_t>(Cxxx) | width);
        }
        else {
            return NONE;
        }
    }

};

#endif //KDL_TYPES_HPP
