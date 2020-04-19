//
// Created by Tom Hancocks on 03/04/2020.
//

#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>
#include "parser/file.hpp"

// MARK: - Helpers

auto kdl::file::exists(std::string_view path) -> bool
{
    struct stat buffer {};
    return (stat(std::string(path).c_str(), &buffer) == 0);
}

auto kdl::file::is_directory(std::string_view path) -> bool
{
    struct stat buffer {};
    return (stat(std::string(path).c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

auto kdl::file::create_directory(std::string_view path) -> void
{
    mkdir(std::string(path).c_str(), 0700);
}

auto kdl::file::create_intermediate(std::string_view path, bool omit_last) -> bool
{
    std::string p(path);

    if (exists(p)) {
        return true;
    }

    // Build a full list of components of the path
    std::vector<std::string> components;
    std::string component;
    while (!p.empty()) {
        component.insert(component.begin(), p.back());
        if (p.back() == '/') {
            components.insert(components.begin(), component);
            component.clear();
        }
        p.pop_back();
    }

    if (omit_last) {
        components.pop_back();
    }

    std::string dir_path;
    for (auto component: components) {
        dir_path.append(component);

        if (is_directory(dir_path)) {
            continue;
        }
        else if (!exists(dir_path)) {
            create_directory(dir_path);
            continue;
        }

        return false;
    }

    return true;
}

auto kdl::file::resolve_tilde(std::string_view path) -> std::string
{
    if (path.length() == 0 || path[0] != '~') {
        return std::string(path);
    }

    std::optional<std::string> home;
    std::size_t pos = path.find_first_of('/');

    if (path.length() == 1 || pos == 1) {
        if (auto value = getenv("HOME")) {
            home = std::string(value);
        }
        else {
            struct passwd *pw = getpwuid(getuid());
            if (pw && pw->pw_dir) {
                home = std::string(pw->pw_dir);
            }
        }
    }
    else {
        std::string user(path, 1, (pos == std::string::npos) ? std::string::npos : pos - 1);
        struct passwd *pw = getpwnam(user.c_str());
        if (pw && pw->pw_dir) {
            home = std::string(pw->pw_dir);
        }
    }

    if (!home.has_value()) {
        return std::string(path);
    }

    std::string result(home.value());
    if (pos == std::string::npos) {
        return result;
    }

    if (result.length() == 0 || result[result.length() - 1] != '/') {
        result.append("/");
    }

    result.append(path.substr(pos + 1));
    return result;
}

auto kdl::file::copy_file(std::string_view src, std::string_view dst) -> void
{
    std::ifstream src_file(src, std::ios::binary);
    std::ofstream dst_file(dst, std::ios::binary);
    dst_file << src_file.rdbuf();
}

// MARK: - Constructors

kdl::file::file()
    : m_path(""), m_contents("")
{

}

kdl::file::file(std::string_view path)
    : m_path(resolve_tilde(path))
{
    if (exists(m_path)) {
        std::ifstream f(m_path);

        // Reserve space in the m_contents string, equivalent to the length of the file.
        f.seekg(0, std::ios::end);
        m_contents.reserve(f.tellg());
        f.seekg(0, std::ios::beg);

        // Read in the contents of the file.
        m_contents.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        m_contents += "\n";
    }
    else {
        m_contents = "";
    }
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
        m_path = resolve_tilde(path.value());
    }

    if (m_path.empty()) {
        return;
    }

    std::ofstream out(m_path);
    out << m_contents;
    out.close();
}
