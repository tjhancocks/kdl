// Copyright (c) 2020 Tom Hancocks
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

#include <iostream>
#include "disassembler/task.hpp"
#include "disassembler/kdl_exporter.hpp"
#include "disassembler/resource_exporter.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "parser/file.hpp"

// MARK: - Construction

kdl::disassembler::task::task(const std::string &destination_dir, std::shared_ptr<target> target)
    : m_destination_dir(destination_dir), m_target(target)
{

}

// MARK: - Root Tasks

auto kdl::disassembler::task::disassemble_resources() -> void
{
    kdl::file::create_directory(m_destination_dir);
    
    for (auto file : graphite::rsrc::manager::shared_manager().files()) {
        std::cout << "Disassembling '" << file->name() << "'" << std::endl;

        // Create a directory for this file.
        auto file_dir = m_destination_dir + "/" + file->name();
        kdl::file::create_directory(file_dir);
        // Iterate through all types registered in KDL, and check if the type exists in the resource file.
        for (auto i = 0; i < m_target->type_container_count(); ++i) {
            auto type_container = m_target->type_container_at(i);

            if (auto type = file->type_container(type_container.code()).lock()) {
                if (type->count() == 0) {
                    continue;
                }

                std::cout << "    - " << type_container.name() << std::endl;
                auto type_dir = file_dir + "/" + type_container.name();
                kdl::file::create_directory(type_dir);

                // Start a KDL exporter for this type.
                kdl_exporter exporter(type_dir + "/" + type_container.name() + "s.kdl");
                exporter.insert_comment("Resource Type Code '" + type_container.code() + "', " + std::to_string(type->count()) + " resources");
                exporter.begin_declaration(type_container.name());

                resource_exporter disassembler(exporter, type_container);
                for (auto resource : type->resources()) {
                    exporter.begin_resource(resource->id(), resource->name());
                    disassembler.disassemble(resource);
                    exporter.end_resource();
                }

                exporter.end_declaration();

                // Conclude the export and save the file.
                exporter.save();
            }
        }
    }
}