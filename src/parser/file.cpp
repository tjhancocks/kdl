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

