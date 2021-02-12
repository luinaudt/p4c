/*
Copyright 2021-present Thomas Luinaud

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdio.h>
#include <string>
#include <iostream>

#include "frontends/common/applyOptionsPragmas.h"
#include "frontends/common/parseInput.h"
#include "frontends/p4/frontend.h"
#include "lib/nullstream.h"
#include "backends/p4fpga/p4fpga.h"
#include "backends/p4fpga/options.h"
#include "backends/p4fpga/version.h"
#include "ir/ir.h"
#include "lib/gc.h"
#include "lib/compile_context.h"
#include "lib/error.h"

int main(int argc, char *const argv[]) {
    setup_gc_logging();
    AutoCompileContext ctxt(new FPGA::FpgaContext);
    auto& options = FPGA::FpgaContext::get().options();
    options.langVersion = CompilerOptions::FrontendVersion::P4_16;
    options.compilerVersion = P4FPGA_VERSION_STRING;
    auto hook = options.getDebugHook();

    if (options.process(argc, argv) != nullptr) {
        options.setInputFile();
    }

    const IR::P4Program *program = nullptr;
    const IR::ToplevelBlock* toplevel = nullptr;

    // program parsing
    program = P4::parseP4File(options);
    if (program == nullptr || ::errorCount() > 0)
        return 1;
    try {
        P4::P4COptionPragmaParser optionsPragmaParser;
        program->apply(P4::ApplyOptionsPragmas(optionsPragmaParser));

        P4::FrontEnd frontend;
        frontend.addDebugHook(hook);
        program = frontend.run(options, program);
    } catch (const std::exception &bug) {
        std::cerr << bug.what() << std::endl;
        return 1;
    }
    if (program == nullptr || ::errorCount() > 0)
        return 1;

    // midend here
    if (options.dumpJsonFile)
        JSONGenerator(*openFile(options.dumpJsonFile, true), true) << program << std::endl;

    // backend
    if (!options.outputFile.isNullOrEmpty()) {
            JSONGenerator(*openFile(options.outputFile, true), true) << program << std::endl;
            /*std::ostream* out = openFile(options.outputFile, false);
            if (out != nullptr) {
                //backend->serialize(*out);
                
                out->flush();
            }*/
        }


    return ::errorCount() > 0;
}