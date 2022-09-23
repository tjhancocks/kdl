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

#include "libraries/spriteworld/spriteworld_library.hpp"

// MARK: - Root Import

auto kdl::builtin::libraries::spriteworld::import(sema::parser &parser) -> void
{
    types::import_rleD(parser);
}

// MARK: - Type : SpriteWorldSprite16 : rlëD

static const std::string s_sprite_type {R"(
@type SpriteWorldSprite16 : "rlëD" {
    template {
        HEXD Data;
    };

    field("PNG") {
        Data as File<PNG> __conversion($InputFormat, rleD);
    };
    field("TGA") {
        Data as File<TGA> __conversion($InputFormat, rleX);
    };
};
)"};

auto kdl::builtin::libraries::spriteworld::types::import_rleD(sema::parser &parser) -> void
{
    parser.import("SpriteWorld.Sprite16", s_sprite_type);
}