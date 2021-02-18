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

#ifndef BACKENDS_FPGA_JSONOBJECTS_H_
#define BACKENDS_FPGA_JSONOBJECTS_H_

#include "lib/json.h"
#include "p4fpga/options.h"
#include <ostream>
namespace FPGA {
    class FPGAJson {
        unsigned outputWidth;
        Util::JsonObject* toplevel;
        public:
            void serialize(std::ostream& out) const {toplevel->serialize(out);};
            FPGAJson(FPGA::P4FpgaOptions& options);
    };
}
#endif // BACKENDS_FPGA_JSONOBJECTS_H_