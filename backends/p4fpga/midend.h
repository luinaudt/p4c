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

#ifndef BACKENDS_P4FPGA_MIDEND_H_
#define BACKENDS_P4FPGA_MIDEND_H_

#include <vector>
#include "frontends/common/resolveReferences/referenceMap.h"
#include "ir/ir.h"
#include "midend/expandEmit.h"
#include "frontends/common/options.h"
#include "frontends/p4/typeMap.h"
#include "midend/convertEnums.h"
#include "backends/p4fpga/packetExternTranslate.h"
#include "backends/p4fpga/staticEval.h"
#include "midend/interpreter.h"

namespace FPGA {
/**
This class implements a policy suitable for the ConvertEnums pass.
The policy is: convert all enums that are not part of the v1model.
Use 32-bit values for all enums.
*/
class EnumOn32Bits : public P4::ChooseEnumRepresentation {
    // TODO
    cstring filename;

    bool convert(const IR::Type_Enum* type) const override {
        if (type->srcInfo.isValid()) {
            auto sourceFile = type->srcInfo.getSourceFile();
            if (sourceFile.endsWith(filename))
                // Don't convert any of the standard enums
                return false;
        }
        return true;
    }
    unsigned enumSize(unsigned) const override
    { return 32; }

 public:
    /// Convert all enums except all the ones appearing in the
    /// specified file.
    explicit EnumOn32Bits(cstring filename) : filename(filename) { }
};


class MidEnd : public PassManager {
 public:
    P4::ReferenceMap    refMap;
    P4::TypeMap         typeMap;
    P4::ConvertEnums::EnumMapping enumMap;
    std::vector<P4::ValueMap*> hdr_status;  // status of all possible hdrs
    const IR::ToplevelBlock   *toplevel = nullptr;

    explicit MidEnd(CompilerOptions& options);
    const IR::ToplevelBlock* process(const IR::P4Program *&program) {
        program = program->apply(*this);
        return toplevel; }
};

}  // namespace FPGA

#endif  // BACKENDS_P4FPGA_MIDEND_H_
