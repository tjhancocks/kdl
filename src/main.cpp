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

#include "parser/file.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "target/target.hpp"
#include "analyzer/template_extractor.hpp"

auto main(int argc, const char **argv) -> int
{
    auto target = std::make_shared<kdl::target>();
    std::vector<std::shared_ptr<kdl::file>> files;

    // Parse the arguments supplied to the program.
    for (auto i = 1; i < argc; ++i) {
        // Check for assembler options
        if (argv[i][0] == '-') {
            std::string arg(argv[i]);

            if (arg == "-tmpl" && i + 2 < argc) {
                // Read in a resource file and build KDL definitions from them.
                std::string res_in(argv[i + 1]);
                std::string dir_out(argv[i + 2]);

                // Send off to the template extractor
                kdl::analyzer::template_extractor extractor(res_in, dir_out);
                extractor.build_type_definitions();

                // Make sure we skip over the parameters.
                i += 2;
            }
        }

        // Anything else should be treated as an input file.
        else {
            files.emplace_back(std::make_shared<kdl::file>(argv[i]));
        }
    }

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

    // Finally save the target to disk.
    target->save();

    return 0;
}