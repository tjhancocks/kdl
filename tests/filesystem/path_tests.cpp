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

#include <libTesting/testing.hpp>
#include <libKDL/host/filesystem/path.hpp>

using namespace kdl::host;

// MARK: - Construction Tests

TEST(file_path_constructWithString)
{
    std::string path_string("/path/to/dir/test.txt");
    filesystem::path path(path_string);
    test::equal(path.component_count(), 4);
    test::equal(path.string(), path_string);
}

TEST(file_path_constructWithComponents)
{
    std::string path_string("/path/to/dir/test.txt");
    filesystem::path path({ "path", "to", "dir", "test.txt" });
    test::equal(path.component_count(), 4);
    test::equal(path.string(), path_string);
}

// MARK: - Component Parsing

TEST(file_path_parseComponentsFromString)
{
    std::string path_string("/path/to/dir/test.txt");
    auto components = filesystem::path::path_components(path_string);
    test::equal(components.size(), 4);
    test::equal(components.at(0), std::string("path"));
    test::equal(components.at(1), std::string("to"));
    test::equal(components.at(2), std::string("dir"));
    test::equal(components.at(3), std::string("test.txt"));
}

// MARK: - String Value

TEST(file_path_toString_expectedResult)
{
    std::string path_string("/path/to/dir/test.txt");
    filesystem::path path(path_string);
    test::equal(path.string(), path_string);
}

// MARK: - Path Relativity

TEST(file_path_isAbsolutePath_startUpVolumeRoot_isTrue)
{
    test::is_true(filesystem::path::is_absolute_path("/System"));
}

TEST(file_path_isAbsolutePath_homeDirectoryTilde_isTrue)
{
    test::is_true(filesystem::path::is_absolute_path("~/Documents"));
}

TEST(file_path_isAbsolutePath_siblingFile_isFalse)
{
    test::is_false(filesystem::path::is_absolute_path("TestDirectory"));
}

// MARK: - Directory

static const std::string test_directory_path { "TestDirectory" };
static const std::string test_file_path { "../README.md" };

TEST(file_path_createDirectory)
{
    filesystem::path::make_directory(filesystem::path(test_directory_path));
}

TEST(file_path_directoryExists)
{
    filesystem::path path(test_directory_path);
    test::is_true(path.exists());
}

TEST(file_path_directoryDoesNotExist)
{
    filesystem::path path(test_directory_path + "_Missing");
    test::is_false(path.exists());
}

TEST(file_path_directoryName_returnsExpectedResult_whenPathItemIsDirectory)
{
    filesystem::path path(test_directory_path);
    test::equal(path.directory_name(), test_directory_path);
}

TEST(file_path_directoryName_returnsExpectedResult_whenPathItemIsFile)
{
#if !TARGET_WINDOWS
    filesystem::path path(test_directory_path);
    test::equal(path.child("Child.txt").directory_name(), test_directory_path);
#else
    // TODO: Fix this test on Windows.
#endif
}

TEST(file_path_name_returnsExpectedResult_whenDirectory)
{
    filesystem::path path(test_directory_path);
    test::equal(path.name(), test_directory_path);
}

TEST(file_path_name_returnsExpectedResult_whenNotDirectory)
{
    std::string child_name = "Child.txt";
    filesystem::path path(test_directory_path);
    test::equal(path.child(child_name).name(), child_name);
}

// MARK: - File Types

TEST(file_path_fileType_returnsExpectedResult_forFile)
{
    filesystem::path file(test_file_path);
    test::equal(file.type(), filesystem::file_type::file);
}

TEST(file_path_fileType_returnsExpectedResult_forDirectory)
{
    filesystem::path dir(test_directory_path);
    test::equal(dir.type(), filesystem::file_type::directory);
}

TEST(file_path_fileType_returnsExpectedResult_forNoneExistentFile)
{
    filesystem::path missing("_PhantomFile");
    test::equal(missing.type(), filesystem::file_type::not_found);
}

TEST(file_path_isDirectory_returnsExpectedResult_forDirectory)
{
    filesystem::path dir(test_directory_path);
    test::is_true(dir.is_directory());
}

TEST(file_path_isDirectory_returnsExpectedResult_forFile)
{
    filesystem::path file(test_file_path);
    test::is_false(file.is_directory());
}

// MARK: - Children

TEST(file_path_childItem_returnsExpectedResult)
{
    std::string child_name = "Child.txt";
    filesystem::path dir(test_directory_path);
    auto file = dir.child(child_name);

    test::equal(file.component_count(), dir.component_count() + 1);
    test::equal(file.name(), child_name);
}