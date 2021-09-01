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

#ifndef BACKENDS_P4FPGA_OPTIONS_H_
#define BACKENDS_P4FPGA_OPTIONS_H_
#include "frontends/common/options.h"
#include <stdlib.h>

namespace FPGA{

class P4FpgaOptions : public CompilerOptions {
 public:
    bool parseOnly = false;
    bool splitDeparser = true;
    cstring outputFile = "a.out";
    unsigned outBusWidth = 64;
    P4FpgaOptions() {
        registerOption("--excludeBackendPasses", "pass1[,pass2]",
                   [this](const char* arg) {
                      excludeBackendPasses = true;
                      auto copy = strdup(arg);
                      while (auto pass = strsep(&copy, ","))
                          passesToExcludeBackend.push_back(pass);
                      return true;
                   },
                   "Exclude passes from backend passes whose name is equal\n"
                   "to one of `passX' strings.\n");
        registerOption("--outputWidth", "width",
                      [this](const char* arg) {
                        outBusWidth = strtoul(arg, nullptr, 10);
                        return true;},
                      "set output bus width in bits. default 64 bits");
        registerOption("--parse-only", nullptr,
                       [this](const char*) {
                           parseOnly = true;
                           return true; },
                       "only parse the P4 input, without any further processing");
        registerOption("--deparser-nosplit", nullptr,
                [this](const char*) { splitDeparser=false; return true; },
                "avoid state split in deparser, simpler graph");
        registerOption("-o", "outfile",
                [this](const char* arg) { outputFile = arg; return true; },
                "Write output to outfile");
     }
};

using FpgaContext = P4CContextWithOptions<P4FpgaOptions>;
}
#endif  // BACKENDS_P4FPGA_OPTIONS_H_
