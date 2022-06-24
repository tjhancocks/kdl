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
#include <libGraphite/data/reader.hpp>
#include <libGraphite/data/writer.hpp>

namespace kdl::media::sound
{

    /**
     * The `kdl::media::sound::wav` class allows reading of WAV audio files
     */
    class wav
    {
    public:
        explicit wav(const std::string& path);
        explicit wav(const graphite::data::block& data);
        wav(std::uint32_t sample_rate, std::uint8_t sample_bits, std::vector<std::vector<uint32_t>> sample_data);

        auto sample_bits() -> std::uint8_t;
        auto sample_rate() -> std::uint32_t;
        auto samples() -> std::vector<std::vector<uint32_t>>;

    protected:
        struct riff_chunk_header {
            uint8_t id[4];
            uint32_t size;
            uint8_t format[4];
        };

        struct fmt_subchunk_header {
            uint8_t id[4];
            uint32_t size;

            uint16_t audio_format;
            uint16_t num_channels;
            uint32_t sample_rate;
            uint32_t byte_rate;
            uint16_t block_align;
            uint16_t bits_per_sample;
        };

        struct data_subchunk_header {
            uint8_t id[4];
            uint32_t size;
            // Data follows
        };

    private:
        std::string m_path;
        uint32_t m_sample_rate;
        uint8_t m_sample_bits;
        std::vector<std::vector<uint32_t>> m_sample_data;

        auto decode(graphite::data::reader& reader) -> bool;
        auto encode(graphite::data::writer& writer) -> void;
        auto find_chunk(graphite::data::reader& reader, uint64_t end_position, std::string chunk_id) -> bool;
    };

}

