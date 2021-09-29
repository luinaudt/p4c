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

#ifndef BACKENDS_P4FPGA_JSONOBJECTS_H_
#define BACKENDS_P4FPGA_JSONOBJECTS_H_

#include "ir/ir.h"
#include "lib/json.h"
#include "p4fpga/options.h"
namespace FPGA {
class FPGAJson {
    Util::JsonObject* toplevel;
    // TODO: support various type of architecture
    Util::JsonObject* parser;
    Util::JsonObject* control;
    Util::JsonObject* deparser;

 public:
    explicit FPGAJson(FPGA::P4FpgaOptions& options);
    void serialize(std::ostream& out) const {
        toplevel->serialize(out);
    };
    void setDeparser(Util::JsonObject* jo);
};
}
#endif  // BACKENDS_P4FPGA_JSONOBJECTS_H_
