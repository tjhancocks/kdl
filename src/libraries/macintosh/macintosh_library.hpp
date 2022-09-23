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

#pragma once

#include "parser/parser.hpp"

namespace kdl::builtin::libraries::macintosh
{
    auto import(sema::parser& parser) -> void;
}

namespace kdl::builtin::libraries::macintosh::types
{
    auto import_string_list(sema::parser& parser) -> void;
    auto import_string(sema::parser& parser) -> void;
    auto import_picture(sema::parser& parser) -> void;
    auto import_color_icon(sema::parser& parser) -> void;
    auto import_sound(sema::parser& parser) -> void;
//    auto import_text(sema::parser& parser) -> void;
//    auto import_ditl(sema::parser& parser) -> void;
//    auto import_dlog(sema::parser& parser) -> void;
//    auto import_tmpl(sema::parser& parser) -> void;
//    auto import_vers(sema::parser& parser) -> void;
}

namespace kdl::builtin::libraries::macintosh::functions
{

}