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

#if !defined(KDL_BINARY_TYPE_HPP)
#define KDL_BINARY_TYPE_HPP

#include <cstdint>
#include <string>

namespace kdl::build_target {

    /**
     * Different data types that might/can be found in a template.
     */
    enum binary_type : uint32_t
    {
        INVALID = 0x00000000,

        // Basic Integer Types
        DBYT = 0x00010000,
        DWRD = 0x00020000,
        DLNG = 0x00030000,
        DQAD = 0x00040000,
        HBYT = 0x00050000,
        HWRD = 0x00060000,
        HLNG = 0x00070000,
        HQAD = 0x00080000,

        // Aligned Integer Types
//        AWRD = 0x00090000,
//        ALNG = 0x000A0000,
//        AQAD = 0x000B0000,

        // Fill Integer Types
//        FBYT = 0x000C0000,
//        FWRD = 0x000D0000,
//        FLNG = 0x000E0000,
//        FQAD = 0x000F0000,

        // Data
        HEXD = 0x00100000,

        // Strings
        PSTR = 0x00110000,
//        LSTR = 0x00120000,
//        WSTR = 0x00130000,
//        ESTR = 0x00140000,
//        OSTR = 0x00150000,
        CSTR = 0x00160000,
        Cnnn = 0x00170000,
//        ECST = 0x00180000,
//        OCST = 0x00190000,

        // Misc
//        BOOL = 0x001A0000,
//        BBIT = 0x001B0000,
//        TNAM = 0x001C0000,
//        CHAR = 0x001D0000,
        RECT = 0x001E0000,
//        Hnnn = 0x001F0000,
//        P0nn = 0x00200000,

        // List support
        OCNT = 0x00210000,
//        LSTZ = 0x00220000,
//        LSTE = 0x00230000,
//        ZCNT = 0x00240000,
//        LSTC = 0x00250000,
//        LSTB = 0x00260000,

    };

    auto binary_type_for_name(const std::string& name) -> enum binary_type;
    auto binary_type_base_size(const enum binary_type& type) -> std::size_t;

};

#endif //KDL_BINARY_TYPE_HPP
