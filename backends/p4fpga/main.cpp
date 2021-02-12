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
#include "backends/p4fpga/p4fpga.h"
#include "backends/p4fpga/options.h"
#include "lib/gc.h"
#include "lib/log.h"
#include "lib/compile_context.h"
#include "backends/p4fpga/version.h"

int main(int argc, char *const argv[]) {
    setup_gc_logging();
    AutoCompileContext ctxt(new FPGA::FpgaContext);
    auto& options = FPGA::FpgaContext::get().options();
    options.langVersion = CompilerOptions::FrontendVersion::P4_16;
    options.compilerVersion = P4FPGA_VERSION_STRING;

    if (options.process(argc, argv) != nullptr) {
        options.setInputFile();
    }

    return 0;
}