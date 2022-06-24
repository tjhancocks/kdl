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

#include <vector>
#include <string>

namespace kdl::installer
{

    struct asset
    {
    public:
        enum flags : std::uint8_t {
            overwrite = 0x01,
            hidden = 0x02,
            directory = 0x04,
            configuration = 0x08,
            copy_file = 0x10,
            intermediates = 0x20,
        };

    public:
        asset(std::string_view name, enum flags flags, std::string_view destination, std::vector<char> data);

        static auto load_assets(std::string_view path) -> std::vector<asset>;

        auto install() const -> void;

    private:
        std::string m_name;
        enum flags m_flags { 0 };
        std::string m_destination;
        std::vector<char> m_data;
    };

}
