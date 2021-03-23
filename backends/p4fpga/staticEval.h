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
#ifndef BACKENDS_FPGA_STATICEVAL_H_
#define BACKENDS_FPGA_STATICEVAL_H_

#include "common/resolveReferences/referenceMap.h"
#include "ir/ir-generated.h"
#include "ir/ir.h"
#include "ir/pass_manager.h"
#include "ir/vector.h"
#include "lib/ordered_map.h"
#include "p4/evaluator/evaluator.h"
#include "p4/typeChecking/typeChecker.h"
#include "p4/typeMap.h"
#include "midend/interpreter.h"
#include <stack>
#include <vector>

namespace FPGA{

class DoStaticEvaluation : public Inspector{
    P4::TypeMap* typeMap;
    P4::ReferenceMap *refMap;
    typedef ordered_map<const IR::StructField*, bool> hdr_value;
    hdr_value* hdr;
    std::vector<hdr_value*> *hdr_vec;
    public:
    /**
    Inspector class for static evaluation of P4 program
    This class goes through header according to execution order
    */
    explicit DoStaticEvaluation(P4::ReferenceMap *refMap, P4::TypeMap *typeMap) :
            typeMap(typeMap), refMap(refMap) {setName("DoStaticEvaluation"); }
    bool preorder(const IR::ToplevelBlock *tlb);
    bool preorder(const IR::P4Parser *block);
    void postorder(const IR::P4Parser *block);
    bool preorder(const IR::P4Control *block);
    void postorder(const IR::P4Control *block);
    bool preorder(const IR::MethodCallStatement *stat);
    bool preorder(const IR::MethodCallExpression *expr);


    private:
    hdr_value* new_hdrMap(){
        return new hdr_value;
    }
    hdr_value* copy_hdrMap(hdr_value* prev){
        auto newMap = new_hdrMap();
        for(auto i: *prev){
            newMap->emplace(i.first, i.second);
        }
        return newMap;
    }
};

class StaticEvaluation : public PassManager{
    public:
    explicit StaticEvaluation(P4::ReferenceMap *refMap, P4::TypeMap *typeMap)
            {
                auto evaluator = new P4::EvaluatorPass(refMap, typeMap);
                auto evaluation = new DoStaticEvaluation(refMap, typeMap);
                passes.push_back(new P4::TypeChecking(refMap, typeMap));
                passes.push_back(evaluator); // we visit from toplevel
                passes.push_back(new VisitFunctor([evaluator, evaluation](){
                    auto toplevel = evaluator->getToplevelBlock();
                    toplevel->apply(*evaluation);
                }));
                //passes.push_back(new DoStaticEvaluation(typeMap));
                setName("StaticEvaluation"); 
            }
};
}
#endif /* BACKENDS_FPGA_STATICEVAL_H_ */