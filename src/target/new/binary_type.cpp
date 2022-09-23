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

#include "target/new/binary_type.hpp"

auto kdl::build_target::binary_type_for_name(const std::string& name) -> enum binary_type
{
    if (name == "HBYT") {
        return binary_type::HBYT;
    }
    else if (name == "DBYT") {
        return binary_type::DBYT;
    }
//    else if (name == "FBYT") {
//        return binary_type::FBYT;
//    }
    else if (name == "HWRD") {
        return binary_type::HWRD;
    }
    else if (name == "DWRD") {
        return binary_type::DWRD;
    }
//    else if (name == "FWRD") {
//        return binary_type::FWRD;
//    }
    else if (name == "HLNG") {
        return binary_type::HLNG;
    }
    else if (name == "DLNG") {
        return binary_type::DLNG;
    }
//    else if (name == "FLNG") {
//        return binary_type::FLNG;
//    }
    else if (name == "HQAD") {
        return binary_type::HQAD;
    }
    else if (name == "DQAD") {
        return binary_type::DQAD;
    }
//    else if (name == "FQAD") {
//        return binary_type::FQAD;
//    }
//    else if (name == "AWRD") {
//        return binary_type::AWRD;
//    }
//    else if (name == "ALNG") {
//        return binary_type::ALNG;
//    }
//    else if (name == "AQAD") {
//        return binary_type::AQAD;
//    }
    else if (name == "HEXD") {
        return binary_type::HEXD;
    }
    else if (name == "PSTR") {
        return binary_type::PSTR;
    }
//    else if (name == "LSTR") {
//        return binary_type::LSTR;
//    }
//    else if (name == "WSTR") {
//        return binary_type::WSTR;
//    }
//    else if (name == "ESTR") {
//        return binary_type::ESTR;
//    }
//    else if (name == "OSTR") {
//        return binary_type::OSTR;
//    }
    else if (name == "CSTR") {
        return binary_type::CSTR;
    }
//    else if (name == "ECST") {
//        return binary_type::ECST;
//    }
//    else if (name == "OCST") {
//        return binary_type::OCST;
//    }
//    else if (name == "BOOL") {
//        return binary_type::BOOL;
//    }
//    else if (name == "BBIT") {
//        return binary_type::BBIT;
//    }
//    else if (name == "TNAM") {
//        return binary_type::TNAM;
//    }
//    else if (name == "CHAR") {
//        return binary_type::CHAR;
//    }
    else if (name == "RECT") {
        return binary_type::RECT;
    }
    else if (name[0] == 'C') {
        auto width = static_cast<uint32_t>(std::stoul(name.substr(1), nullptr, 16));
        return static_cast<binary_type>(static_cast<uint32_t>(Cnnn) | width);
    }
//    else if (name[0] == 'H') {
//        auto width = static_cast<uint32_t>(std::stoul(name.substr(1), nullptr, 16));
//        return static_cast<binary_type>(static_cast<uint32_t>(Hnnn) | width);
//    }
//    else if (name[0] == 'P' && name[1] == '0') {
//        auto width = static_cast<uint32_t>(std::stoul(name.substr(1), nullptr, 16));
//        return static_cast<binary_type>(static_cast<uint32_t>(P0nn) | width);
//    }
    else if (name == "OCNT") {
        return binary_type::OCNT;
    }
//    else if (name == "LSTZ") {
//        return binary_type::LSTZ;
//    }
    else if (name == "LSTE") {
        return binary_type::LSTE;
    }
//    else if (name == "ZCNT") {
//        return binary_type::ZCNT;
//    }
    else if (name == "LSTC") {
        return binary_type::LSTC;
    }
//    else if (name == "LSTB") {
//        return binary_type::LSTB;
//    }
    else if (name == "RSRC") {
        return binary_type::RSRC;
    }

    return binary_type::INVALID;
};

auto kdl::build_target::binary_type_base_size(enum kdl::build_target::binary_type type) -> std::size_t
{
    switch (type & ~0xFFF) {
        case binary_type::CSTR:
        case binary_type::PSTR:
        case binary_type::HBYT:
        case binary_type::DBYT: {
            return 1;
        }
        case binary_type::HWRD:
        case binary_type::DWRD:
        case binary_type::OCNT: {
            return 2;
        }
        case binary_type::HLNG:
        case binary_type::DLNG: {
            return 4;
        }
        case binary_type::RECT:
        case binary_type::HQAD:
        case binary_type::DQAD: {
            return 8;
        }
        case binary_type::RSRC: {
            return 9;
        }
        case binary_type::Cnnn: {
            return type & 0xFFF;
        }
        case binary_type::HEXD: {
            return 0;
        }
        default:
            return 0;
    }
}
