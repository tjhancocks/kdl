//
// Created by Tom Hancocks on 03/04/2020.
//

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
         */
        auto save(std::optional<std::string> path = {}) -> void;

    private:
        std::string m_path;
        uint8_t *m_raw;
        uint64_t m_length;

    };

};
