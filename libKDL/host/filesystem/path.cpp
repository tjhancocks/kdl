// Copyright (c) 2022 Tom Hancocks
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

#include <libKDL/util/availability.hpp>
#include <libKDL/host/filesystem/path.hpp>

#if TARGET_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <sys/stat.h>

// MARK: - Helpers

auto kdl::host::filesystem::path::path_components(const std::string& path, char separator) -> std::vector<std::string>
{
    std::vector<std::string> components;
    std::string component;
    std::string p(path);

    while (!p.empty()) {
        if (p.back() == separator) {
            components.insert(components.begin(), component);
            component.clear();
        }
        else {
            component.insert(component.begin(), p.back());
        }
        p.pop_back();
    }

    if (!component.empty()) {
        components.insert(components.begin(), component);
        component.clear();
    }

    return std::move(components);
}

// MARK: - Constructors

kdl::host::filesystem::path::path(const std::string& str)
    : m_components(path_components(str)), m_relative(!is_absolute_path(str))
{
    convert_to_absolute();
}

kdl::host::filesystem::path::path(const std::initializer_list<std::string>& components, bool is_relative)
    : m_components(components), m_relative(is_relative)
{
    convert_to_absolute();
}

kdl::host::filesystem::path::path(const std::vector<std::string>& components, bool is_relative)
    : m_components(components), m_relative(is_relative)
{
    convert_to_absolute();
}

// MARK: - Absolute Path Conversion

auto kdl::host::filesystem::path::convert_to_absolute() -> void
{
    if (!m_relative) {
        return;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == nullptr) {
        switch (errno) {
            case EACCES: {
                throw std::runtime_error("Access Denied");
            }
            case ENOMEM: {
                throw std::runtime_error("Insufficient Storage");
            }
            default : {
                throw std::runtime_error("Unrecognised Error: " + std::to_string(errno));
            }
        }
    }

    std::string working_directory(cwd);

#if TARGET_WINDOWS
    char separator = '\\';
#else
    char separator = '/';
#endif

    // Common functionality. Split the working directory into its components and add them to the beginning
    // of the path.
    auto components = path_components(working_directory, separator);
    m_components.insert(m_components.begin(), components.begin(), components.end());
    m_relative = false;
}

// MARK: - Accessors

auto kdl::host::filesystem::path::string() const -> std::string
{
    if (m_path_buffer.empty()) {
        std::string result;
        for (const auto& component : m_components) {
            result.insert(result.end(), '/');
            result.insert(result.end(), component.begin(), component.end());
        }
        if (m_relative && result[0] == '/') {
            result.erase(0, 1);
        }
        const_cast<path *>(this)->m_path_buffer = std::move(result);
    }
    return m_path_buffer;
}

auto kdl::host::filesystem::path::c_str() const -> const char *
{
    (void)string();
    return m_path_buffer.c_str();
}

auto kdl::host::filesystem::path::directory_name() const -> std::string
{
    auto it = m_components.rbegin();
    if (!is_directory()) {
        it++;
    }
    return *it;
}

auto kdl::host::filesystem::path::name() const -> std::string
{
    return *m_components.rbegin();
}

auto kdl::host::filesystem::path::type() const -> file_type
{
    if (!exists(*this)) {
        return file_type::not_found;
    }
    else if (is_directory()) {
        return file_type::directory;
    }
    else {
        return file_type::file;
    }
}

auto kdl::host::filesystem::path::is_directory() const -> bool
{
    return is_directory(*this);
}

auto kdl::host::filesystem::path::exists() const -> bool
{
    return exists(*this);
}

auto kdl::host::filesystem::path::component_count() const -> std::size_t
{
    return m_components.size();
}

// MARK: - Children

auto kdl::host::filesystem::path::child(const std::string &name) const -> path
{
    auto components = m_components;
    components.insert(components.end(), name);
    return filesystem::path(components, m_relative);
}

// MARK: - Static Helpers

auto kdl::host::filesystem::path::is_absolute_path(const std::string &path) -> bool
{
    return !path.empty() && (path[0] == '/' || path[0] == '~');
}

auto kdl::host::filesystem::path::exists(const path &path) -> bool
{
    struct stat buffer {};
    return (stat(resolve_tilde(path).c_str(), &buffer) == 0);
}

auto kdl::host::filesystem::path::is_directory(const path &path) -> bool
{
#if TARGET_WINDOWS
    auto result = GetFileAttributesA(path.c_str());
    return ((result & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
    struct stat buffer {};
    return (stat(resolve_tilde(path).c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
#endif
}

auto kdl::host::filesystem::path::resolve_tilde(const path &path) -> filesystem::path
{
#if TARGET_WINDOWS
    return path;
#else
    std::string path_str = path.string();
    if (path_str.empty() || path_str[0] != '~') {
        return path;
    }

    // TODO: This could be refactored to make use of the components in the path.
    std::string home;
    std::size_t pos = path_str.find_first_of('/');

    if (path_str.length() == 1 || pos == 1) {
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
        std::string user(path_str, 1, (pos == std::string::npos) ? std::string::npos : pos - 1);
        struct passwd *pw = getpwnam(user.c_str());
        if (pw && pw->pw_dir) {
            home = std::string(pw->pw_dir);
        }
    }

    if (!home.empty()) {
        return filesystem::path(path_str);
    }

    std::string result(home);
    if (pos == std::string::npos) {
        return filesystem::path(result);
    }

    if (result.length() == 0 || result[result.length() - 1] != '/') {
        result.append("/");
    }

    result.append(path_str.substr(pos + 1));
    return filesystem::path(result);
#endif
}

// MARK: - File Item Creation

auto kdl::host::filesystem::path::touch() -> void
{
    // TODO: Implement touch functionality...
}

// MARK: - Directory Creation

auto kdl::host::filesystem::path::make_directory(const path &path) -> void
{
#if TARGET_WINDOWS
    CreateDirectory(path.c_str(), nullptr);
#else
    mkdir(path.c_str(), 0700);
#endif
}

auto kdl::host::filesystem::path::create_directory(bool ignore_last) -> bool
{
    auto components = m_components;
    if (ignore_last) {
        components.pop_back();
    }

    std::string dir_path_str;
    for (const auto& component : m_components) {
        dir_path_str.append(component);
        filesystem::path dir_path(dir_path_str);

        if (is_directory(dir_path)) {
            continue;
        }
        else if (!exists(dir_path)) {
            make_directory(dir_path);
            continue;
        }

        return false;
    }

    return true;
}