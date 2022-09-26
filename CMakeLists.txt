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

########################################################################################################################
## Project Configuration
project(KDL)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

set(CMAKE_BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_BINARY_DIR ${CMAKE_BUILD_DIR})
set(CMAKE_OUTPUT_PATH ${CMAKE_BUILD_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(PROJECT_SUBMODULE_DIR ${PROJECT_SOURCE_DIR}/Submodules)

########################################################################################################################
## Graphite
if (NOT TARGET Graphite)
	add_subdirectory(${PROJECT_SUBMODULE_DIR}/Graphite)
endif()

########################################################################################################################
## libKDL - Main Executable
file(GLOB_RECURSE libKDL_sources
	libKDL/*.cpp
)
add_library(kdl ${libKDL_sources})
target_include_directories(kdl PUBLIC
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_SUBMODULE_DIR}/Graphite"
)
target_link_libraries(kdl Graphite)

########################################################################################################################
## KDL - Main Executable
file(GLOB_RECURSE kdl_sources
	src/*.cpp
) 
add_executable(kdl-bin ${kdl_sources})
target_include_directories(kdl-bin PUBLIC
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SUBMODULE_DIR}/Graphite"
	"${CMAKE_BUILD_DIR}"
)
target_link_libraries(kdl-bin Graphite)
set_property(TARGET kdl-bin PROPERTY XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
set_target_properties(kdl-bin PROPERTIES OUTPUT_NAME kdl)

########################################################################################################################
## KDL - Version
find_package(Git)
add_custom_target(generate-kdl-version
		${CMAKE_COMMAND} -D SRC=${PROJECT_SOURCE_DIR}/src/kdl_version.hpp.in
						 -D DST=${CMAKE_BUILD_DIR}/kdl_version.hpp
						 -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
						 -P ${PROJECT_SOURCE_DIR}/Support/CMake/GenerateVersionHeader.cmake
)
add_dependencies(kdl generate-kdl-version)

########################################################################################################################
## KDL - Test Suite
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/KDLTests.cmake)
	include(${PROJECT_SUBMODULE_DIR}/Graphite/libTesting/testing.cmake)
	build_testing_library()
	add_testing_target(kdl ${CMAKE_CURRENT_SOURCE_DIR}/tests)
	include(${CMAKE_CURRENT_SOURCE_DIR}/KDLTests.cmake)
endif()