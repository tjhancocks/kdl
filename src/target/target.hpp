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

#if !defined(KDL_TARGET_HPP)
#define KDL_TARGET_HPP

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "disassembler/task.hpp"
#include "target/new/type_container.hpp"
#include "target/new/resource_instance.hpp"
#include "libGraphite/rsrc/file.hpp"
#include "target/track/resource_tracking.hpp"

namespace kdl
{

    /**
     * The kdl::target class represents the output target of a KDL assembler pass.
     *
     * All resources will ultimately be written to this target.
     */
    class target : public std::enable_shared_from_this<target>
    {
    private:
        std::string m_name { "Untitled Project" };
        std::string m_version { "1.0" };
        std::vector<std::string> m_authors {};
        std::string m_dst_root;
        std::string m_dst_file;
        std::string m_src_root;
        std::string m_scenario_root;
        graphite::rsrc::file::format m_format { graphite::rsrc::file::format::classic };
        std::optional<graphite::rsrc::file::format> m_required_format{};
        std::vector<build_target::type_container> m_type_containers;
        graphite::rsrc::file m_file;
        std::shared_ptr<kdl::resource_tracking::table> m_resource_tracking_table {};

        std::optional<disassembler::task> m_disassembler;
        std::vector<lexeme> m_disassembler_image_format { lexeme("PNG", lexeme::identifier) };
        std::vector<lexeme> m_disassembler_sound_format { lexeme("WAV", lexeme::identifier) };

        auto target_file_path() const -> std::string;

    public:
        target();

        [[nodiscard]] auto file() -> graphite::rsrc::file&;

        auto set_project_name(const std::string& name) -> void;
        auto add_author(const std::string& name) -> void;
        auto set_version_string(const std::string& version) -> void;

        auto set_dst_path(const std::string& dst_path) -> void;

        auto set_scenario_root(const std::string& path) -> void;
        auto scenario_manifest(std::string_view scenario_name) -> std::string;

        auto set_format(const std::string& format) -> void;
        auto set_required_format(const graphite::rsrc::file::format& format) -> bool;

        auto set_src_root(const std::string& src_root) -> void;
        auto resolve_src_path(const std::string& path) const -> std::string;

        auto add_type_container(const build_target::type_container& container) -> void;
        auto type_container_count() const -> std::size_t;
        auto type_container_at(const int& i) const -> build_target::type_container;
        auto type_container_named(const kdl::lexeme& name) const -> build_target::type_container;

        auto add_resource(const build_target::resource_instance& resource) -> void;

        auto set_disassembler_image_format(const std::vector<lexeme>& formats) -> void;
        auto set_disassembler_sound_format(const std::vector<lexeme>& formats) -> void;
        auto initialise_disassembler(const std::string& output_dir) -> void;
        auto disassembler() const -> std::optional<disassembler::task>;

        auto resource_tracker() const -> std::shared_ptr<kdl::resource_tracking::table>;

        auto save() -> void;
    };

};

#endif //KDL_TARGET_HPP
