# Copyright (c) 2019 Tom Hancocks
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.0)

project(KDL)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 17)

# libGraphite - Dependancy
include_directories("${PROJECT_SOURCE_DIR}/Submodules/Graphite")
file(GLOB_RECURSE graphite_sources
	Submodules/Graphite/libGraphite/*.cpp
)
add_library(Graphite ${graphite_sources})

# kdl - Main Executable
include_directories(
	"${PROJECT_SOURCE_DIR}/src"
	"${CMAKE_BINARY_DIR}"
)

file(GLOB_RECURSE kdl_sources
	src/*.cpp
) 
add_executable(kdl ${kdl_sources})

find_package(Git)
add_custom_target(version
		${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/src/kdl_version.hpp.in
						 -D DST=${CMAKE_BINARY_DIR}/kdl_version.hpp
						 -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
						 -P ${CMAKE_SOURCE_DIR}/Support/CMake/GenerateVersionHeader.cmake
)
add_dependencies(kdl version)

target_link_libraries(kdl Graphite)

# kdl - Tests
enable_testing()
file(GLOB_RECURSE kdl_examples
	Support/Examples/*.kdl
)
foreach(example ${kdl_examples})
    string(REGEX REPLACE ".*/" "" test_name "${example}")
    string(REGEX REPLACE ".kdl$" "" test_name "${test_name}")
    add_test(
    	NAME ${test_name} 
    	COMMAND "${EXECUTABLE_OUTPUT_PATH}/kdl" "${example}"
    	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endforeach(example)
add_custom_command(
	TARGET kdl
    POST_BUILD
    COMMAND ctest --output-on-failure
)
