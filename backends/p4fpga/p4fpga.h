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

#ifndef BACKENDS_P4FPGA_P4FPGA_H_
#define BACKENDS_P4FPGA_P4FPGA_H_

#include <vector>
#include "backends/p4fpga/JsonObjects.h"
#include "ir/ir.h"
#include "lib/json.h"
#include "backends/p4fpga/options.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "frontends/p4/typeMap.h"
#include "midend/convertEnums.h"
#include "midend/interpreter.h"
#include "p4fpga/staticEval.h"


namespace FPGA {
class FPGABackend : public PassManager{
    P4FpgaOptions& options;
    FPGA::FPGAJson* json;
    unsigned busOutWidth;
    P4::ReferenceMap*    refMap;
    P4::TypeMap*         typeMap;
    P4::ConvertEnums::EnumMapping enumMap;
    std::vector<ValueMapList*>* hdr_status;
    const IR::ToplevelBlock* tlb;

 public:
    void serialize(std::ostream& out) const{
        json->serialize(out);
    };
    explicit FPGABackend(P4FpgaOptions& options,
                P4::ReferenceMap* refMap, P4::TypeMap* typeMap,
                std::vector<ValueMapList*> *hdr_vec);

    void convert(const IR::P4Program *&program);
};


}  // namespace FPGA

#endif  // BACKENDS_P4FPGA_P4FPGA_H_
