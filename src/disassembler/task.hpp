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

#pragma once

#include <string>
#include <memory>
#include <optional>
#include "parser/lexeme.hpp"

namespace kdl { class target; }

namespace kdl::disassembler
{

    class task
    {
    public:
        task(const std::string& destination_dir, std::shared_ptr<target> target);

        auto set_preferred_image_formats(const std::vector<lexeme>& formats) -> void;
        auto set_preferred_sound_formats(const std::vector<lexeme>& formats) -> void;

        [[nodiscard]] auto format_priority(const lexeme& format) const -> int;
        [[nodiscard]] auto appropriate_conversion_format(const lexeme& input, int priority) const -> std::optional<lexeme>;
        [[nodiscard]] auto format_extension(const lexeme& format) const -> std::string;

        auto disassemble_resources() -> void;

    private:
        std::string m_destination_dir;
        std::vector<lexeme> m_preferred_image_export_format {};
        std::vector<lexeme> m_preferred_sound_export_format {};
        std::shared_ptr<target> m_target;

    };

}
