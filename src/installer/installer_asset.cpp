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

#include <iostream>
#include "installer/installer_asset.hpp"
#include "libGraphite/rsrc/file.hpp"
#include "libGraphite/data/reader.hpp"
#include "parser/file.hpp"

// MARK: - Constructor

kdl::installer::asset::asset(std::string_view name, enum asset::flags flags, std::string_view destination, std::vector<char> data)
    : m_name(name), m_flags(flags), m_destination(destination), m_data(data)
{

}

// MARK: - Asset Loading

auto kdl::installer::asset::load_assets(std::string_view path) -> std::vector<asset>
{
    std::vector<asset> assets;

    auto resolved_path = std::string(path);
    graphite::rsrc::file file(resolved_path);

    if (auto container = file.type_container("ASET").lock()) {
        for (auto raw_asset : container->resources()) {
            graphite::data::reader reader(raw_asset->data());

            auto flags = static_cast<enum asset::flags>(reader.read_byte());
            auto destination = reader.read_pstr();
            auto data = reader.read_bytes(reader.size() - reader.position());

            installer::asset asset(raw_asset->name(), flags, destination, data);
            assets.emplace_back(asset);
        }
    }
    else {
        // TODO: Appropriate notification to the user that the installer was bad?
    }

    return assets;
}

auto kdl::installer::asset::install() const -> void
{
    auto dst_path = kdl::file::resolve_tilde(m_destination);

    // Determine what action to take.
    if (m_flags & directory) {
        // Creating a directory. Make sure its safe to create the directory
        if (kdl::file::exists(dst_path)) {
            if (!kdl::file::is_directory(dst_path)) {
                throw std::runtime_error("Unable to create directory '" + m_destination + "'. File already exists.");
            }
            // The directory already exists.
            return;
        }

        // Nothing exists, create it.
        std::cout << m_name << std::endl;
        kdl::file::create_directory(dst_path);
    }
    else if (m_flags & copy_file) {
        // Copying a file from the source location to the destination.
        std::cout << m_name << std::endl;

        std::string raw_src_path(m_data.begin(), m_data.end());
        kdl::file::copy_file(kdl::file::resolve_tilde(raw_src_path), dst_path);
    }
    else {
        // We're creating a new file.
        if (!(m_flags & overwrite) && kdl::file::exists(dst_path)) {
            // File already exists and we're not allowed to overwrite it so stop here.
            // This is not an error!
            return;
        }

        std::cout << m_name << std::endl;

        if (m_flags & intermediates) {
            // We need to create any intermediate directories on path.
            if (!kdl::file::create_intermediate(dst_path)) {
                throw std::runtime_error("Unable to construct intermediate directories for '" + m_destination + "'");
            }
        }

        kdl::file file;
        file.set_contents(std::string(m_data.begin(), m_data.end()));
        file.save(dst_path);
    }
}
