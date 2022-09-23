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

#include "libraries/macintosh/macintosh_library.hpp"

// MARK: - Root Import

auto kdl::builtin::libraries::macintosh::import(sema::parser &parser) -> void
{
    types::import_string_list(parser);
    types::import_string(parser);
    types::import_picture(parser);
    types::import_color_icon(parser);
    types::import_sound(parser);
}

// MARK: - Type : StringList : STR#

static const std::string s_string_list_type {R"(
@type StringList : "STR#" {
    template {
        OCNT Strings;
        LSTC StringsBegin;
        PSTR    String;
        LSTE StringsEnd;
    };

    field("String") repeatable<0, 32767, Strings> {
        String;
    };
};
)"};

auto kdl::builtin::libraries::macintosh::types::import_string_list(sema::parser &parser) -> void
{
    parser.import("Macintosh.StringList", s_string_list_type);
}

// MARK: - Type : String: STR

static const std::string s_string_type {R"(
@type String : "STR " {
    template {
        CSTR String;
        HEXD Data;
    };

    field("String") {
        String as File;
    };
};
)"};

auto kdl::builtin::libraries::macintosh::types::import_string(sema::parser &parser) -> void
{
    parser.import("Macintosh.String", s_string_type);
}

// MARK: - Type : Picture: PICT

static const std::string s_picture_type {R"(
@type Picture : "PICT" {
    template {
        HEXD Image;
    };

    field("PNG") {
        Image as File<PNG> __conversion($InputFormat, PICT);
    };
    field("TGA") {
        Image as File<TGA> __conversion($InputFormat, PICT);
    };
};
)"};

auto kdl::builtin::libraries::macintosh::types::import_picture(sema::parser &parser) -> void
{
    parser.import("Macintosh.Picture", s_picture_type);
}

// MARK: - Type : Picture: PICT

static const std::string s_color_icon_type {R"(
@type ColorIcon : "cicn" {
    template {
        HEXD Image;
    };

    field("PNG") {
        Image as File<PNG> __conversion($InputFormat, PICT);
    };
    field("TGA") {
        Image as File<TGA> __conversion($InputFormat, PICT);
    };
};
)"};

auto kdl::builtin::libraries::macintosh::types::import_color_icon(sema::parser &parser) -> void
{
    parser.import("Macintosh.ColorIcon", s_color_icon_type);
}

// MARK: - Type : Picture: PICT

static const std::string s_sound_type {R"(
@type Sound : "snd " {
    template {
        HEXD Image;
    };

    field("WAV") {
        Image as File<WAV> __conversion($InputFormat, snd);
    };
};
)"};

auto kdl::builtin::libraries::macintosh::types::import_sound(sema::parser &parser) -> void
{
    parser.import("Macintosh.Sound", s_sound_type);
}