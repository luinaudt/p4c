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

#ifndef BACKENDS_FPGA_OPTIONS_H_
#define BACKENDS_FPGA_OPTIONS_H_
#include "frontends/common/options.h"

namespace FPGA{

class P4FpgaOptions : public CompilerOptions {
 public:
    bool parseOnly = false;
    cstring outputFile = "a.out";
    P4FpgaOptions() {
        registerOption("--parse-only", nullptr,
                       [this](const char*) {
                           parseOnly = true;
                           return true; },
                       "only parse the P4 input, without any further processing");
        registerOption("-o", "outfile",
                [this](const char* arg) { outputFile = arg; return true; },
                "Write output to outfile");
     }
};

using FpgaContext = P4CContextWithOptions<P4FpgaOptions>;
}
#endif