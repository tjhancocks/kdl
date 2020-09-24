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
#include "kdl_version.hpp"
#include "parser/file.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "target/target.hpp"
#include "analyzer/template_extractor.hpp"
#include "installer/installer_asset.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "disassembler/task.hpp"
#include "generation/lua/lua_generator.hpp"

auto main(int argc, const char **argv) -> int
{
    auto target = std::make_shared<kdl::target>();
    std::vector<std::shared_ptr<kdl::file>> files;

    // Load in the default system configuration.
    // TODO: The configuration file should be located in a different location on Windows.
    if (kdl::file::exists("~/.config.kdl")) {
        auto configuration_file = std::make_shared<kdl::file>("~/.config.kdl");
        target->set_src_root(configuration_file->path());
        kdl::sema::parser(target, kdl::lexer(configuration_file).analyze()).parse();
    }

    // Parse the arguments supplied to the program.
    for (auto i = 1; i < argc; ++i) {
        // Check for assembler options
        if (argv[i][0] == '-') {
            std::string arg(argv[i]);

            if (arg == "--configuration") {
                // Specify a configuration file to load over the top of the previous configuration.
                std::string configuration_path(argv[i + 1]);
                auto configuration_file = std::make_shared<kdl::file>(configuration_path);
                target->set_src_root(configuration_file->path());
                kdl::sema::parser(target, kdl::lexer(configuration_file).analyze()).parse();
                i += 1;
            }
            else if (arg == "--install") {
                std::string installer_path(argv[i + 1]);
                i += 1;

                // TODO: Extract to its own class/method.
                for (auto asset : kdl::installer::asset::load_assets(installer_path)) {
                    asset.install();
                }
            }
            else if (arg == "-v" || arg == "--version") {
                // Display the version information to the user
                std::cout << "KDL Version " << KDL_VERSION << std::endl;
                std::cout << "\t" << KDL_LICENSE << " " << KDL_AUTHORS << std::endl;
            }
            else if (arg == "-o") {
                // Set the output destination. This output destination could be either a singular file,
                // or a directory. If it is a file then extract the file name. It needs to be separated
                // from the output directory.
                target->set_dst_path(argv[i + 1]);

                // Make we skip over the parameter.
                i += 1;
            }
            else if (arg == "-s" || arg == "--scenario") {
                // Look up the scenario and load it.
                std::string scenario_name(argv[i + 1]);
                i += 1;
                
                auto scenario_manifest = target->scenario_manifest(scenario_name);
                auto manifest_file = std::make_shared<kdl::file>(scenario_manifest);
                target->set_src_root(manifest_file->path());
                kdl::sema::parser(target, kdl::lexer(manifest_file).analyze()).parse();
            }
            else if (arg == "-i" || arg == "--include") {
                // Look up the data file referenced and read it into the resource manager.
                auto file = std::make_shared<graphite::rsrc::file>(kdl::file::resolve_tilde(std::string(argv[i + 1])));
                i += 1;

                graphite::rsrc::manager::shared_manager().import_file(file);
            }
            else if (arg == "-d" || arg == "--disassemble") {
                target->initialise_disassembler(kdl::file::resolve_tilde(std::string(argv[++i])));
            }
            else if (arg == "-tmpl" && i + 2 < argc) {
                // Read in a resource file and build KDL definitions from them.
                std::string res_in(argv[i + 1]);
                std::string dir_out(argv[i + 2]);

                // Send off to the template extractor
                kdl::analyzer::template_extractor extractor(res_in, dir_out);
                extractor.build_type_definitions();

                // Make sure we skip over the parameters.
                i += 2;
            }
            else if (arg == "--generate-lua") {
                // Generate Lua code files with classes/definitions for use in Kestrel that are based upon the
                // type definitions.
                // The first argument is the name of the type that we want to generate lua for.
                // The second argument is the output directory.
                std::string type_name(argv[i + 1]);
                std::string dir_out(kdl::file::resolve_tilde(std::string(argv[i + 2])));

                std::cout << "Generating Lua Definition for '" << type_name << "'" << std::endl;

                // Find the type.
                auto container = target->type_container_named({ type_name, kdl::lexeme::identifier });
                kdl::generator::lua_type generator(container, dir_out);
                generator.generate();

                // Make sure we advance over the parameters
                i += 2;
            }
        }

        // Anything else should be treated as an input file.
        else {
            files.emplace_back(std::make_shared<kdl::file>(argv[i]));
        }
    }

    if (!files.empty()) {
        // Loop through each of the files and parse them.
        for (auto file : files) {
            // 0. Configure the target.
            target->set_src_root(file->path());

            // 1. Perform lexical analysis.
            kdl::lexer lexer(file);
            auto lexemes = lexer.analyze();

            // 2. Parse the lexical analysis result.
            kdl::sema::parser parser(target, lexemes);
            parser.parse();
        }
    }

    // Finally save the target to disk, if there are resources present in it.
    if (target->type_container_count() > 0) {
        target->save();
    }

    // Perform disassembly if a disassembly option has been specified.
    if (target->disassembler().has_value()) {;
        target->disassembler()->disassemble_resources();
    }

    return 0;
}