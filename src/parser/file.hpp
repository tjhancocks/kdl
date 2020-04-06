//
// Created by Tom Hancocks on 03/04/2020.
//

#if !defined(KDL_FILE_HPP)
#define KDL_FILE_HPP

#include <string>
#include <optional>

namespace kdl
{

    /**
     * The kdl::file structure represents a KDL source file. This file could have been read
     * from disk, or be in the process of being constructed.
     */
    struct file
    {
    private:
        std::string m_path;
        std::string m_contents;

    public:
        /**
         * Create a blank file for writing to.
         */
        file();

        /**
         * Read the specified file from disk.
         * @param path The path from with the load the file.
         */
        file(const std::string path);

        /**
         * The location of the file on disk. Will be empty if this is a blank file.
         * @return A string representing a file path.
         */
        auto path() const -> std::string;

        /**
         * The contents of the file as they currently are.
         * @return A reference to the contents of the file.
         */
        auto contents() -> std::string&;

        /**
         * Set the contents of the file without saving the changes to disk.
         * @param contents The new contents of the file.
         */
        auto set_contents(const std::string contents) -> void;

        /**
         * Save the contents of the file to disk.
         */
        auto save(std::optional<std::string> path = {}) -> void;

    };

};

#endif //KDL_FILE_HPP
