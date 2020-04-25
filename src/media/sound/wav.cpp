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

#include <iostream>
#include "wav.hpp"

// MARK: - Constructors

kdl::media::sound::wav::wav(const std::string path)
{
    graphite::data::reader reader(path);

    // TODO: Possibly handle any errors that occur?
    decode(reader);
}

kdl::media::sound::wav::wav(std::shared_ptr<std::vector<char>> data)
    : m_sample_rate(1), m_sample_bits(8)
{
    auto ptr = std::make_shared<graphite::data::data>(data, data->size(), 0, graphite::data::data::lsb);
    graphite::data::reader reader(ptr, 0);
    decode(reader);
}

kdl::media::sound::wav::wav(uint32_t sample_rate, uint8_t sample_bits, std::vector<std::vector<uint32_t>> sample_data)
    : m_sample_rate(sample_rate), m_sample_bits(sample_bits), m_sample_data(sample_data)
{

}

// MARK: - Decoding

auto kdl::media::sound::wav::find_chunk(graphite::data::reader& reader, uint64_t end_position, std::string chunk_id) -> bool
{
    while (reader.position() + 4 < end_position) {
        auto chunk_type = reader.read_cstr(4);
        if (chunk_type != chunk_id) {
            auto chunk_size = reader.read_long();
            reader.move(chunk_size);
        } else {
            return true;
        }
    }
    return false;
}

auto kdl::media::sound::wav::decode(graphite::data::reader &reader) -> bool
{
    if (reader.read_cstr(4) != "RIFF") {
        std::cerr << "WAV Decoder: Expected 'RIFF' chunk" << std::endl;
        return false;
    }

    auto end_position = reader.read_long() + reader.position();

    if (reader.read_cstr(4) != "WAVE") {
        std::cerr << "WAV Decoder: Expected 'WAVE' format" << std::endl;
        return false;
    }

    if (!find_chunk(reader, end_position, "fmt ")) {
        std::cerr << "WAV Decoder: Expected 'fmt ' subchunk" << std::endl;
        return false;
    }

    fmt_subchunk_header fmt;
    fmt.size = reader.read_long();
    fmt.audio_format = reader.read_short();
    if (fmt.audio_format != 1 || fmt.size != 16) {
        std::cerr << "WAV Decoder: Expected PCM file (format 1, size 16; got format " + std::to_string(fmt.audio_format) +
                     ", size " + std::to_string(fmt.size) + ")" << std::endl;
        return false;
    }

    fmt.num_channels = reader.read_short();
    fmt.sample_rate = reader.read_long();
    fmt.byte_rate = reader.read_long();
    fmt.block_align = reader.read_short();
    fmt.bits_per_sample = reader.read_short();

    if (!find_chunk(reader, end_position, "data")) {
        std::cerr << "WAV Decoder: Expected 'fmt ' subchunk" << std::endl;
        return false;
    }

    auto data_size = reader.read_long();
    auto num_samples = data_size / fmt.block_align;
    auto bytes_per_sample = fmt.bits_per_sample / 8;
    auto sample_offset = fmt.bits_per_sample == 8 ? 0 : (1 << (fmt.bits_per_sample - 1));

    m_sample_bits = fmt.bits_per_sample;
    m_sample_rate = fmt.sample_rate;
    m_sample_data.resize(fmt.num_channels, std::vector<uint32_t>(num_samples));

    for (auto s = 0; s < num_samples; s++) {
        auto sample_start = reader.position();

        for (auto c = 0; c < fmt.num_channels; c++) {
            m_sample_data[c][s] = 0;
            for (auto b = 0; b < bytes_per_sample; b++) {
                m_sample_data[c][s] |= reader.read_byte() << (b * 8);
            }
            if (bytes_per_sample > 1 && m_sample_data[c][s] > sample_offset) {
                // Sign-extend for any value above 8 bits
                m_sample_data[c][s] |= 0xFFFFFFFF << fmt.bits_per_sample;
            }
            // Add the sample offset to map the signed range onto the equivalent unsigned range
            m_sample_data[c][s] = static_cast<int64_t>(m_sample_data[c][s]) + sample_offset;
        }

        // Move forward by the block align byte count -- may be different to the actual sample bytes
        reader.set_position(sample_start + fmt.block_align);
    }

    // Finished
    return true;
}

// MARK: - Encoding

auto kdl::media::sound::wav::encode(graphite::data::writer &writer) -> void
{
    writer.data()->set_byte_order(graphite::data::data::byte_order::lsb);

    auto num_channels = m_sample_data.size();
    if (!num_channels) {
        return;
    }

    auto num_samples = m_sample_data[0].size();
    auto sample_bytes = m_sample_bits / 8;
    auto sample_offset = 1 << (m_sample_bits - 1);

    // Header
    writer.write_cstr("RIFF", 4);
    writer.write_long(36 + sample_bytes * num_channels * num_samples);
    writer.write_cstr("WAVE", 4);

    // FMT subchunk
    writer.write_cstr("fmt ", 4);
    writer.write_long(16); // SubChunk1Size
    writer.write_short(1); // PCM is format 1
    writer.write_short(num_channels);
    writer.write_long(m_sample_rate);
    writer.write_long(m_sample_rate * num_channels * sample_bytes); // byte rate
    writer.write_short(num_channels * sample_bytes); // block align
    writer.write_short(m_sample_bits);

    // data subchunk
    writer.write_cstr("data", 4);
    writer.write_long(m_sample_bits / 8 * num_samples * num_channels);
    for (auto s = 0; s < num_samples; s++) {
        for (auto c : m_sample_data) {
            int32_t sample = static_cast<int64_t>(c[s]) - sample_offset;

            if (m_sample_bits == 8) {
                // Byte range is 0 -> 255, midpoint 128
                writer.write_byte(static_cast<uint8_t>(c[s]));
            }
            else if (m_sample_bits == 16) {
                // Range -32768 -> 32767, midpoint 0
                writer.write_signed_short(sample);
            }
            else if (m_sample_bits == 24) {
                // Range -8338608 -> 8338607, midpoint 0
                writer.write_byte(sample & 0xFF);
                writer.write_byte((sample >> 8) & 0xFF);
                writer.write_byte((sample >> 16) & 0xFF);
            }
            else if (m_sample_bits == 32) {
                // Range -2147483648 -> 2147483647, midpoint 0
                writer.write_signed_long(sample);
            }
        }
    }
}

// MARK: - Accessors

auto kdl::media::sound::wav::sample_bits() -> uint8_t {
    return m_sample_bits;
}

auto kdl::media::sound::wav::sample_rate() -> uint32_t {
    return m_sample_rate;
}

auto kdl::media::sound::wav::samples() -> std::vector<std::vector<uint32_t>> {
    return m_sample_data;
}
