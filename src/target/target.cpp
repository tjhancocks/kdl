// Copyright (c) 2019-2020 Tom Hancocks
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
#include "target/target.hpp"
#include "diagnostic/fatal.hpp"
#include "parser/file.hpp"

// MARK: - Constructors

kdl::target::target()
    : m_dst_root("."), m_dst_file("result"), m_resource_tracking_table(std::make_shared<kdl::resource_tracking::table>())
{

}

// MARK: - Metadata Management

auto kdl::target::set_project_name(const std::string& name) -> void
{
    m_name = name;
}

auto kdl::target::add_author(const std::string& name) -> void
{
    m_authors.emplace_back(name);
}

auto kdl::target::set_version_string(const std::string& version) -> void
{
    m_version = version;
}


// MARK: - Container Management

auto kdl::target::add_type_container(const build_target::type_container& container) -> void
{
    m_type_containers.emplace_back(container);
}

auto kdl::target::type_container_count() const -> std::size_t
{
    return m_type_containers.size();
}

auto kdl::target::type_container_at(const int& i) const -> build_target::type_container
{
    return m_type_containers[i];
}

auto kdl::target::type_container_named(const kdl::lexeme& name) const -> build_target::type_container
{
    for (auto c : m_type_containers) {
        if (c.name() == name.text()) {
            return c;
        }
    }
    log::fatal_error(name, 1, "Missing definition for type '" + name.text() + "'");
}


// MARK: - Destination Paths

auto kdl::target::set_dst_path(const std::string& dst_path) -> void
{
    auto path = dst_path;
    std::string filename;

    if (kdl::file::exists(path) && !kdl::file::is_directory(path)) {
        while (path.substr(path.size() - 1) != "/") {
            filename.insert(0, path.substr(path.size() - 1));
            path.pop_back();
        }
    }
    else if (kdl::file::exists(path) && kdl::file::is_directory(path)) {
        filename = "result";
    }
    else {
        while (!path.empty() && path.substr(path.size() - 1) != "/") {
            filename.insert(0, path.substr(path.size() - 1));
            path.pop_back();
        }

        if (!path.empty() && !kdl::file::exists(path)) {
            kdl::file::is_directory(path);
        }
    }

    // Make sure the end of the path is a name.
    if (!path.empty() && path.substr(path.size() - 1) == "/") {
        path.pop_back();
    }

    m_dst_root = path;
    m_dst_file = filename;
}

// MARK: - Scenario Paths

auto kdl::target::set_scenario_root(const std::string& path) -> void
{
    m_scenario_root = std::string(path);

    if (m_scenario_root.substr(m_scenario_root.size() - 1) == "/") {
        m_scenario_root.pop_back();
    }
}

auto kdl::target::scenario_manifest(std::string_view scenario_name) -> std::string
{
    auto path = kdl::file::resolve_tilde(m_scenario_root) + "/" + std::string(scenario_name);
    if (!kdl::file::exists(path) && !kdl::file::is_directory(path)) {
        std::cerr << "Could not find scenario named: '" + std::string(scenario_name) + "'." << std::endl;
        exit(1);
    }

    path.append("/manifest.kdl");
    if (!kdl::file::exists(path)) {
        std::cerr << "Scenario '" + std::string(scenario_name) + "' is missing a 'manifest.kdl' file." << std::endl;
        exit(1);
    }

    return path;
}

// MARK: - Source Paths

auto kdl::target::set_src_root(const std::string& src_root) -> void
{
    auto path = src_root;
    // This needs to be a directory. Check if the path provided is a KDL file. If it is truncate the file name.
    // If there is a terminating /, then truncate it too.
    if (path.substr(path.size() - 4) == ".kdl") {
        while (path.substr(path.size() - 1) != "/") {
            path.pop_back();
        }
    }

    if (path.substr(path.size() - 1) == "/") {
        path.pop_back();
    }

    // Save the path.
    m_src_root = path;
}

auto kdl::target::resolve_src_path(const std::string& path) const -> std::string
{
    // TODO: Improve this so it actually makes more sense.
    std::string rpath("@rpath"); // Root Path (Location of Input File)
    std::string spath("@spath"); // Scenario Path (Location of Scenario Type Definitions)
    std::string opath("@opath"); // Output Path (Location of Target Output)

    if (path.substr(0, rpath.size()) == rpath) {
        return m_src_root + path.substr(rpath.size());
    }
    if (path.substr(0, spath.size()) == spath) {
        return m_src_root + path.substr(spath.size());
    }
    if (path.substr(0, opath.size()) == opath) {
        return m_src_root + path.substr(opath.size());
    }

    return path;
}

// MARK: - Resource Formats

auto kdl::target::set_format(const std::string &format) -> void
{
    if (format == "extended") {
        m_format = graphite::rsrc::file::extended;
    }
    else if (format == "classic") {
        m_format = graphite::rsrc::file::classic;
    }
    else if (format == "rez") {
        m_format = graphite::rsrc::file::rez;
    }
    else {
        std::cerr << "Unrecognised resource file format specified: " << format << std::endl;
        exit(2);
    }

    if (!set_required_format(m_format)) {
        std::cerr << "Unable to use the '" << format << "' resource format. One or more KDL files require a different format." << std::endl;
        exit(3);
    }
}

auto kdl::target::set_required_format(const graphite::rsrc::file::format &format) -> bool
{
    // If there is already a required resource format then check if it matches the new format.
    if (m_required_format.has_value() && m_required_format.value() != format) {
        return false;
    }
    m_required_format = format;

    return true;
}

// MARK: - Resource Management

auto kdl::target::add_resource(const build_target::resource_instance& resource) -> void
{
    m_resource_tracking_table->add_instance(m_file.name(), resource.type_code(), resource.id(), resource.name());
    m_file.add_resource(resource.type_code(),
                        resource.id(),
                        resource.name(),
                        resource.assemble(),
                        resource.attributes());
}

// MARK: - Saving

auto kdl::target::target_file_path() const -> std::string
{
    auto path = m_dst_root;

    if (!path.empty() && path.substr(path.size() - 1) != "/") {
        path += "/";
    }
    path += m_dst_file;

    switch (m_required_format.has_value() ? m_required_format.value() : m_format) {
        case graphite::rsrc::file::format::classic:
            path += ".ndat";
            break;
        case graphite::rsrc::file::format::extended:
            path += ".kdat";
            break;
        case graphite::rsrc::file::format::rez:
            path += ".rez";
            break;
    }

    return path;
}

auto kdl::target::save() -> void
{
    std::cout << "saving to " << target_file_path() << std::endl;
    m_file.write(target_file_path(), m_format);
}

// MARK: - Disassembler

auto kdl::target::set_disassembler_image_format(const std::vector<lexeme>& formats) -> void
{
    m_disassembler_image_format = formats;
}

auto kdl::target::set_disassembler_sound_format(const std::vector<lexeme>& formats) -> void
{
    m_disassembler_sound_format = formats;
}

auto kdl::target::initialise_disassembler(const std::string& output_dir) -> void
{
    m_disassembler = kdl::disassembler::task(output_dir, shared_from_this());
    m_disassembler->set_preferred_image_formats(m_disassembler_image_format);
    m_disassembler->set_preferred_sound_formats(m_disassembler_sound_format);
}

auto kdl::target::disassembler() const -> std::optional<disassembler::task>
{
    return m_disassembler;
}

// MARK: - Resource Tracker

auto kdl::target::resource_tracker() const -> std::shared_ptr<kdl::resource_tracking::table>
{
    return m_resource_tracking_table;
}

// MARK: - Global Variables

auto kdl::target::set_global_variable(const std::string& var_name, const kdl::lexeme &value) -> void
{
    m_globals.insert(std::make_pair(var_name, value));
}

auto kdl::target::global_variable(const std::string& var_name) const -> std::optional<kdl::lexeme>
{
    for (const auto& var : m_globals) {
        if (var.first == var_name) {
            return var.second;
        }
    }
    return {};
}

auto kdl::target::all_global_variables() const -> std::map<std::string, kdl::lexeme>
{
    return m_globals;
}
