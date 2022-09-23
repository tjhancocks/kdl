//
// Created by Tom Hancocks on 03/04/2020.
//

#include <sys/stat.h>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>
#include <cctype>
#include <sstream>
#include <functional>
#include <cstring>
#include "parser/file.hpp"

#if (_WIN32 || _WIN64)
    // Windows Specific
#   include <windows.h>
#else
    // Linux / macOS Specific
#   define USE_GLOB
#   include <glob.h>
#   include <pwd.h>
#   include <unistd.h>
#endif

// MARK: - Prototypes

int alphanum_impl(const char *l, const char *r);

// MARK: - Helpers

auto kdl::file::exists(std::string_view path) -> bool
{
    struct stat buffer {};
    return (stat(resolve_tilde(path).c_str(), &buffer) == 0);
}

auto kdl::file::is_directory(std::string_view path) -> bool
{
#if (_WIN32 || _WIN64)
    auto result = GetFileAttributesA(std::string(path).c_str());
    return (result & FILE_ATTRIBUTE_DIRECTORY);
return false;
#else
    struct stat buffer {};
    return (stat(resolve_tilde(path).c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
#endif
}

auto kdl::file::create_directory(std::string_view path) -> void
{
#if (_WIN32 || _WIN64)
    CreateDirectory(std::string(path).c_str(), NULL);
#else
    mkdir(std::string(path).c_str(), 0700);
#endif
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

#if (_WIN32 || _WIN64)
auto kdl::file::resolve_tilde(std::string_view path) -> std::string
{
    return std::string(path);
}
#else
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
#endif

auto kdl::file::copy_file(std::string_view src, std::string_view dst) -> void
{
    std::ifstream src_file(std::string(src), std::ios::binary);
    std::ofstream dst_file(std::string(dst), std::ios::binary);
    dst_file << src_file.rdbuf();
}

auto kdl::file::glob(const std::string& path) -> std::shared_ptr<std::vector<std::string>>
{
    auto files = std::make_shared<std::vector<std::string>>();

#if defined(USE_GLOB)
    glob_t result;
    int err = ::glob(path.c_str(), GLOB_ERR | GLOB_MARK, NULL, &result);

    for (auto f = 0; f < result.gl_pathc; f++) {
        std::string fpath(result.gl_pathv[f]);
        if (fpath.back() != '/') {  // Skip directories
            files->emplace_back(fpath);
        }
    }

    globfree(&result);
#else
    std::cout << "Note: File globbing is not currently supported in the Windows version of KDL" << std::endl;
    files->emplace_back(path);
#endif

    std::sort(files->begin(), files->end(), [&](const std::string& left, const std::string& right) -> bool {
        std::ostringstream l; l << left;
        std::ostringstream r; r << right;
        return alphanum_impl(l.str().c_str(), r.str().c_str()) < 0;
    });

    return files;
}

// MARK: - Constructors

kdl::file::file()
    : m_path(""), m_raw(nullptr), m_length(0)
{

}

kdl::file::file(std::string_view path)
    : m_path(resolve_tilde(path))
{
    if (exists(m_path)) {
        std::ifstream f(m_path, std::ios::binary);

        // Reserve space in the m_contents string, equivalent to the length of the file.
        f.seekg(0, std::ios::end);
        m_length = static_cast<uint64_t>(f.tellg());
        m_raw = new uint8_t[m_length + 1];
        memset(m_raw, 0, m_length);
        f.seekg(0, std::ios::beg);

        // Read in the contents of the file.
        f.read((char *)m_raw, m_length);
        m_raw[m_length++] = '\n';

        f.close();
    }
    else {
        m_raw = nullptr;
    }
}

kdl::file::file(const std::string &name, const std::string &contents)
    : m_path(name)
{
    m_length = contents.size();
    m_raw = new uint8_t[m_length + 1];
    memset(m_raw, 0, m_length);
    memcpy(m_raw, contents.c_str(), m_length);
}

// MARK: - Destructors

kdl::file::~file()
{
    delete[] m_raw;
}

// MARK: - Accessors

auto kdl::file::exists() const -> bool
{
    return (m_raw != nullptr);
}

auto kdl::file::path() const -> std::string
{
    return m_path;
}

auto kdl::file::contents() -> std::string
{
    std::string s;
    s.assign(m_raw, m_raw + m_length);
    return s;
}

auto kdl::file::vector() -> std::vector<char>
{
    std::vector<char> v(m_raw, m_raw + m_length);
    return v;
}

auto kdl::file::set_contents(const std::string& contents) -> void
{
    if (m_raw) {
        delete[] m_raw;
    }

    m_length = contents.size();
    m_raw = new uint8_t[m_length];
    memcpy(m_raw, contents.c_str(), m_length);
}

// MARK: - Saving

auto kdl::file::save(std::optional<std::string> path) -> void
{
    // TODO: This needs to be a binary file save rather than text based.
    if (path.has_value()) {
        m_path = resolve_tilde(path.value());
    }

    if (m_path.empty()) {
        return;
    }

    std::ofstream out(m_path);
    out << contents();
    out.close();
}

// MARK: - Alphanum Sort Implementation

/*
The Alphanum Algorithm is an improved sorting algorithm for strings
containing numbers.  Instead of sorting numbers in ASCII order like a
standard sort, this algorithm sorts numbers in numeric order.
The Alphanum Algorithm is discussed at http://www.DaveKoelle.com
This implementation is Copyright (c) 2008 Dirk Jagdmann <doj@cubic.org>.
It is a cleanroom implementation of the algorithm and not derived by
other's works. In contrast to the versions written by Dave Koelle this
source code is distributed with the libpng/zlib license.
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you
             must not claim that you wrote the original software. If you use
             this software in a product, an acknowledgment in the product
             documentation would be appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and
             must not be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
             distribution. */

/**
     compare l and r with strcmp() semantics, but using
     the "Alphanum Algorithm". This function is designed to read
     through the l and r strings only one time, for
     maximum performance. It does not allocate memory for
     substrings. It can either use the C-library functions isdigit()
     and atoi() to honour your locale settings, when recognizing
     digit characters when you "#define ALPHANUM_LOCALE=1" or use
     it's own digit character handling which only works with ASCII
     digit characters, but provides better performance.
     @param l NULL-terminated C-style string
     @param r NULL-terminated C-style string
     @return negative if l<r, 0 if l equals r, positive if l>r
 */
int alphanum_impl(const char *l, const char *r)
{
    enum mode_t { STRING, NUMBER } mode = STRING;

    while (*l && *r) {
        if (mode == STRING) {
            char l_char, r_char;
            while ((l_char = *l) && (r_char = *r)) {
                // check if this are digit characters
                const bool l_digit = isdigit(l_char), r_digit = isdigit(r_char);
                // if both characters are digits, we continue in NUMBER mode
                if (l_digit && r_digit) {
                    mode = NUMBER;
                    break;
                }
                // if only the left character is a digit, we have a result
                if (l_digit) return -1;
                // if only the right character is a digit, we have a result
                if (r_digit) return +1;
                // compute the difference of both characters
                const int diff = l_char - r_char;
                // if they differ we have a result
                if (diff != 0) return diff;
                // otherwise process the next characters
                ++l;
                ++r;
            }
        }
        else { // mode==NUMBER
            // get the left number
            char *end;
            unsigned long l_int = strtoul(l, &end, 0);
            l = end;

            // get the right number
            unsigned long r_int = strtoul(r, &end, 0);
            r = end;

            // if the difference is not equal to zero, we have a comparison result
            const long diff = l_int - r_int;
            if (diff != 0) return diff;

            // otherwise we process the next substring in STRING mode
            mode = STRING;
        }
    }

    if (*r) return -1;
    if (*l) return +1;
    return 0;
}
