// Copyright (c) 2019-2022 Tom Hancocks
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
#include <vector>
#include <memory>
#include <optional>

namespace kdl
{

    /**
     * The kdl::file structure represents a KDL source file. This file could have been read
     * from disk, or be in the process of being constructed.
     */
    struct file
    {
    public:
        static auto exists(std::string_view path) -> bool;
        static auto is_directory(std::string_view path) -> bool;
        static auto create_directory(std::string_view path) -> void;
        static auto create_intermediate(std::string_view path, bool omit_last = true) -> bool;
        static auto resolve_tilde(std::string_view path) -> std::string;
        static auto copy_file(std::string_view src, std::string_view dst) -> void;
        static auto glob(const std::string& path) -> std::shared_ptr<std::vector<std::string>>;

    public:
        /**
         * Create a blank file for writing to.
         */
        file();

        ~file();

        /**
         * Read the specified file from disk.
         * @param path The path from with the load the file.
         */
        explicit file(std::string_view path);

        /**
         * Create an in memory file,
         */
        file(const std::string& name, const std::string& contents);

        [[nodiscard]] auto exists() const -> bool;

        /**
         * The location of the file on disk. Will be empty if this is a blank file.
         * @return A string representing a file path.
         */
        [[nodiscard]] auto path() const -> std::string;

        /**
         * The contents of the file as a string
         */
        auto contents() -> std::string;

        /**
         * Set the contents of the file without saving the changes to disk.
         * @param contents The new contents of the file.
         */
        auto set_contents(const std::string& contents) -> void;

        /**
         * The contents of the file as a vector.
         */
        auto vector() -> std::vector<char>;

        /**
         * Save the contents of the file to disk.
         * @param path     The location of the file in which the file contents should be saved to.
         */
        auto save(const std::string& path = "") -> void;

    private:
        std::string m_path;
        std::uint8_t *m_raw { nullptr };
        std::uint64_t m_length { 0 };
    };

};
