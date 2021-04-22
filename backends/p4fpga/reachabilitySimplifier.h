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
#ifndef BACKENDS_P4FPGA_REACHABILITYSIMPLIFIER_H_
#define BACKENDS_P4FPGA_REACHABILITYSIMPLIFIER_H_

#include <vector>
#include "ir/ir.h"
#include "ir/visitor.h"
#include "midend/interpreter.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "p4/evaluator/evaluator.h"
#include "p4/typeChecking/typeChecker.h"
#include "p4/typeMap.h"


namespace FPGA {
class doReachabilitySimplifier : public Transform{
    P4::P4CoreLibrary&     corelib;
    P4::ReferenceMap* refMap;
    P4::TypeMap* typeMap;
    P4::ExpressionEvaluator* evaluator;
    const P4::SymbolicValueFactory* factory;
    std::vector<P4::ValueMap*> *hdr_vec;

 public:
    const IR::Node* preorder(IR::P4Program* prog) override;
    const IR::Node* preorder(IR::P4Control* ctrl) override;
    const IR::Node* preorder(IR::IfStatement* cond) override;
    const IR::Node* postorder(IR::BlockStatement* block) override;

    explicit doReachabilitySimplifier(P4::ReferenceMap* refMap, P4::TypeMap* typeMap,
                                 std::vector<P4::ValueMap *> *hdr_status)
    :  corelib(P4::P4CoreLibrary::instance), refMap(refMap), typeMap(typeMap),
        hdr_vec(hdr_status){
            visitDagOnce = false;
            CHECK_NULL(refMap);
            CHECK_NULL(typeMap);
            CHECK_NULL(hdr_status);
            factory = new P4::SymbolicValueFactory(typeMap);
            setName("doReachabilitySimplifier");
        }
};
class ReachabilitySimplifier : public PassManager{
 public:
    explicit ReachabilitySimplifier(P4::ReferenceMap* refMap, P4::TypeMap* typeMap,
                                 std::vector<P4::ValueMap *> *hdr_status)
            {
                auto depReduce = new doReachabilitySimplifier(refMap, typeMap, hdr_status);
                passes.push_back(new P4::TypeChecking(refMap, typeMap));
                passes.push_back(depReduce);
                setName("reachabilitySimplifier");
            }
};
}  // namespace FPGA

#endif  // BACKENDS_P4FPGA_REACHABILITYSIMPLIFIER_H_
