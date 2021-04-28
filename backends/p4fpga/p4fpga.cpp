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

#include "backends/p4fpga/p4fpga.h"
#include "common/options.h"
#include "ir/ir.h"
#include "backends/p4fpga/deparser.h"
#include "backends/p4fpga/deparserGraphCloser.h"
#include "p4/evaluator/evaluator.h"
#include "p4fpga/reachabilitySimplifier.h"

namespace FPGA{
FPGABackend::FPGABackend(FPGA::P4FpgaOptions& options,
                    P4::ReferenceMap* refMap, P4::TypeMap* typeMap,
                    std::vector<ValueMapList*> *hdr_vec) :
                options(options), refMap(refMap), typeMap(typeMap),
                hdr_status(hdr_vec){
        json = new FPGA::FPGAJson(options);
        busOutWidth = options.outBusWidth;
        auto evaluator = new P4::EvaluatorPass(refMap, typeMap);
        addPasses({
            new doDeparserGraphCloser(refMap, typeMap),
            new doReachabilitySimplifier(refMap,
                                         typeMap,
                                         hdr_status->at(4))
        });
    }
void FPGABackend::convert(const IR::P4Program *&program){
    program = program->apply(*this);
    LOG2("end passes");
    auto mainDecls = program->getDeclsByName(IR::P4Program::main)->toVector();
    auto main = mainDecls->at(0)->to<IR::Declaration_Instance>();
    auto deparser = main->arguments->at(main->arguments->size()-1);
    auto depType = deparser->expression->type->getP4Type();

    if (!main) return;
    auto depConv = new DeparserConverter(json, refMap, typeMap);
    LOG2(deparser);
    int pos=0;
    for (auto i : program->objects) {
        if (i->is<IR::P4Control>()){
            auto block = i->to<IR::P4Control>();
            // we only visit deparsers.
            if (block->type->getP4Type()->equiv(*depType->getNode())) {
                block->apply(*depConv);
            }
        }
        pos++;
    }
}

}  // Namespace FPGA
