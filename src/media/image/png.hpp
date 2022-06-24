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

#pragma once

#include <vector>
#include <libGraphite/data/data.hpp>
#include <libGraphite/quickdraw/support/surface.hpp>

namespace kdl::media::image
{

    /**
     * The `kdl::media::image::png` class allows reading/writing and working with PNG
     * images.
     */
    class png
    {
    public:
        explicit png(const std::string& path);
        explicit png(const graphite::data::block& data);
        explicit png(graphite::quickdraw::surface& surface);

        [[nodiscard]] auto surface() const -> const graphite::quickdraw::surface&;
        [[nodiscard]] auto data() const -> graphite::data::block;

    private:
        std::string m_path;
        graphite::quickdraw::surface m_surface;

        auto decode(graphite::data::reader& reader) -> bool;
        auto encode(graphite::data::writer& writer) const -> void;

    };

}
