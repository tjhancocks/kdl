//
// Created by Tom Hancocks on 03/04/2020.
//

#include <fstream>
#include <streambuf>
#include <iostream>
#include "parser/file.hpp"

// MARK: - Constructors

kdl::file::file()
    : m_path(""), m_contents("")
{

}

kdl::file::file(const std::string path)
    : m_path(path)
{
    std::ifstream f(m_path);

    // Reserve space in the m_contents string, equivalent to the length of the file.
    f.seekg(0, std::ios::end);
    m_contents.reserve(f.tellg());
    f.seekg(0, std::ios::beg);

    // Read in the contents of the file.
    m_contents.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    m_contents += "\n";
}

// MARK: - Accessors

auto kdl::file::path() const -> std::string
{
    return m_path;
}

auto kdl::file::contents() -> std::string&
{
    return m_contents;
}

auto kdl::file::set_contents(const std::string contents) -> void
{
    m_contents = contents;
}

// MARK: - Saving

auto kdl::file::save(std::optional<std::string> path) -> void
{
    if (path.has_value()) {
        m_path = path.value();
    }

    if (m_path.empty()) {
        return;
    }

    std::ofstream out(m_path);
    out << m_contents;
    out.close();
}