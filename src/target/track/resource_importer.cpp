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

#include <utility>
#include "target/track/resource_importer.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Construction

kdl::resource_tracking::importer::importer(std::string code, const int64_t &id)
    : m_code(std::move(code)), m_id(id)
{

}

// MARK: - Importer

auto kdl::resource_tracking::importer::populate(kdl::build_target::resource_instance &instance) -> bool
{
    // Find and load the resource.
    if (auto res = graphite::rsrc::manager::shared_manager().find(m_code, m_id).lock()) {
        // We need the type template of the resource in order to parse out the binary data into the instance.
        auto tmpl = instance.get_type_template();
        graphite::data::reader reader(res->data());

        for (auto i = 0; i < tmpl.binary_field_count(); ++i) {
            auto field = tmpl.binary_field_at(i);

            // The data type of the field will tell us how to read the next segment of data from the binary
            // resource data.
            switch (field.type & ~0xFFFUL) {
                case kdl::build_target::binary_type::HBYT: {
                    instance.write(i, reader.read_byte());
                    break;
                }
                case kdl::build_target::binary_type::HWRD: {
                    instance.write(i, reader.read_short());
                    break;
                }
                case kdl::build_target::binary_type::HLNG: {
                    instance.write(i, reader.read_long());
                    break;
                }
                case kdl::build_target::binary_type::HQAD: {
                    instance.write(i, reader.read_quad());
                    break;
                }
                case kdl::build_target::binary_type::DBYT: {
                    instance.write(i, reader.read_signed_byte());
                    break;
                }
                case kdl::build_target::binary_type::DWRD: {
                    instance.write(i, reader.read_signed_short());
                    break;
                }
                case kdl::build_target::binary_type::DLNG: {
                    instance.write(i, reader.read_signed_long());
                    break;
                }
                case kdl::build_target::binary_type::DQAD: {
                    instance.write(i, reader.read_signed_quad());
                    break;
                }
                case kdl::build_target::binary_type::RECT: {
                    instance.write(i, std::tuple<int16_t, int16_t, int16_t, int16_t>(
                            reader.read_signed_short(),
                            reader.read_signed_short(),
                            reader.read_signed_short(),
                            reader.read_signed_short()
                    ));
                    break;
                }
                case kdl::build_target::binary_type::PSTR: {
                    instance.write(i, std::tuple<std::size_t, std::string>(
                            reader.read_byte(0, graphite::data::reader::peek),
                            reader.read_pstr()
                    ));
                    break;
                }
                case kdl::build_target::binary_type::Cnnn: {
                    auto length = field.type & 0xFFFUL;
                    instance.write(i, std::tuple<std::size_t, std::string>(
                        length, reader.read_cstr(length)
                    ));
                    break;
                }
                case kdl::build_target::binary_type::CSTR: {
                    instance.write(i, std::tuple<std::size_t, std::string>(0, reader.read_cstr()));
                    break;
                }
                default: {
                    return false;
                }
            }
        }

        // At this point we have successfully transposed all of the binary data into the resource instance.
        return true;
    }
    return false;
}