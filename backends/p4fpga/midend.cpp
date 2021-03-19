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
#include "common/constantFolding.h"
#include "ir/ir-generated.h"
#include "ir/pass_manager.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "lib/indent.h"
#include "lib/log.h"
#include "midend/fillEnumMap.h"
#include "midend/flattenHeaders.h"
#include "midend/eliminateNewtype.h"
#include "midend/parserUnroll.h"
#include "midend/local_copyprop.h"
#include "midend/predication.h"
#include "midend/midEndLast.h"
#include "frontends/p4/simplifyParsers.h"
#include "p4/moveDeclarations.h"
#include "p4/simplify.h"
#include "p4/typeMap.h"
#include "p4fpga/emitCond.h"

namespace FPGA {
    MidEnd::MidEnd(CompilerOptions& options){
        // temporary
        auto convertEnums = new P4::ConvertEnums(&refMap, &typeMap, new EnumOn32Bits("v1model.p4"));        // evaluator compiler-design.pptx slide 77
        auto evaluator = new P4::EvaluatorPass(&refMap, &typeMap);
        std::initializer_list<Visitor *> midendPasses = {
            new P4::EliminateNewtype(&refMap, &typeMap),
            convertEnums, new VisitFunctor([this, convertEnums]() { enumMap = convertEnums->getEnumMapping(); }),
            new P4::ResolveReferences(&refMap),
            new P4::TypeChecking(&refMap, &typeMap),
            new P4::SimplifyParsers(&refMap),
            new P4::ExpandEmit(&refMap, &typeMap),
            new P4::TypeChecking(&refMap, &typeMap),
            new EmitCond(&refMap, &typeMap),
            new P4::FlattenHeaders(&refMap, &typeMap),
            new P4::MoveDeclarations(),  // more may have been introduced
            new P4::ConstantFolding(&refMap, &typeMap),
            new P4::LocalCopyPropagation(&refMap, &typeMap),
            new P4::ConstantFolding(&refMap, &typeMap),
            new P4::SimplifyControlFlow(&refMap, &typeMap),
            evaluator,
            new VisitFunctor([this, evaluator]() { // set toplevel
                             toplevel = evaluator->getToplevelBlock(); }),
            new StaticEvaluation(&refMap, &typeMap),
            new P4::MidEndLast(),
            evaluator,
            new VisitFunctor([this, evaluator]() { // set toplevel
                                toplevel = evaluator->getToplevelBlock(); }) 
        };
        addPasses(midendPasses);
        if (options.excludeMidendPasses) {
            removePasses(options.passesToExcludeMidend);
        }
    }
}