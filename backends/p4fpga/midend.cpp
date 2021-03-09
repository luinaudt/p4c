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
#include "backends/p4fpga/midend.h"
#include "ir/pass_manager.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "midend/fillEnumMap.h"
#include "p4fpga/emitCond.h"

namespace FPGA {
    MidEnd::MidEnd(CompilerOptions& options){
        // temporary
        auto convertEnums = new P4::ConvertEnums(&refMap, &typeMap, new EnumOn32Bits("v1model.p4"));        // evaluator compiler-design.pptx slide 77
        auto evaluator = new P4::EvaluatorPass(&refMap, &typeMap);
        std::initializer_list<Visitor *> midendPasses = {
         /*   new P4::ResolveReferences(&refMap),
            new P4::TypeChecking(&refMap, &typeMap),
            convertEnums,
            new VisitFunctor([this, convertEnums]() { enumMap = convertEnums->getEnumMapping(); }),*/
            new P4::ExpandEmit(&refMap, &typeMap),
            new EmitCond(&refMap, &typeMap),
            evaluator,
            new VisitFunctor([this, evaluator]() { // set toplevel
                                toplevel = evaluator->getToplevelBlock(); }) 
        };
        addPasses(midendPasses);

    }

}