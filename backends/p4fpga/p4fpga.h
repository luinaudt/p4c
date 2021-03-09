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

#ifndef P4FPGA_H_
#define P4FPGA_H_

#include "backends/p4fpga/JsonObjects.h"
#include "ir/ir.h"
#include "lib/json.h"
#include "backends/p4fpga/options.h"
#include <cstring>
#include <ostream>
#include "frontends/common/resolveReferences/referenceMap.h"
#include "frontends/p4/typeMap.h"
#include "midend/convertEnums.h"


namespace FPGA {
    class FPGABackend {
        P4FpgaOptions& options;
        FPGA::FPGAJson* json;
        unsigned busOutWidth;
        P4::ReferenceMap*    refMap;
        P4::TypeMap*         typeMap;
        P4::ConvertEnums::EnumMapping enumMap;
        
    public:
        void serialize(std::ostream& out) const {json->serialize(out);};
        FPGABackend(P4FpgaOptions& options, 
                    P4::ReferenceMap* refMap, P4::TypeMap* typeMap);
        void convert(const IR::ToplevelBlock* tlb);
    };


}  // namespace FPGA

#endif 