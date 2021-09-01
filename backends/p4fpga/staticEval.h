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
#ifndef BACKENDS_P4FPGA_STATICEVAL_H_
#define BACKENDS_P4FPGA_STATICEVAL_H_

#include <stack>
#include <vector>
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

namespace FPGA{

class ValueMapList : public std::vector<P4::ValueMap*>{
 public:
    using std::vector<P4::ValueMap*>::vector;
     /**
    insert val into list, check if there is no duplicate
    */
    void push_unique(P4::ValueMap* val);
    void merge(ValueMapList* val);
    ValueMapList* remove_duplicate();
    /**
    reset all hdr to match a new control block
    */
    ValueMapList* update_hdr_ref(const IR::Parameter* hdrParam);
    ValueMapList* clone(){
        return new ValueMapList(*this);
    };
};

class DoStaticEvaluation : public Inspector{
    P4::TypeMap*              typeMap;
    P4::ReferenceMap*         refMap;
    P4::ValueMap*             hdr;
    std::stack<P4::ValueMap*> hdr_stack;
    P4::ExpressionEvaluator*  evaluator;
    ValueMapList*             hdr_vec;
    ValueMapList*             hdr_vecIn;
    std::vector<ValueMapList*> *hdr_vec_list;
    const P4::SymbolicValueFactory* factory;

 public:
    /**
    Inspector class for static evaluation of P4 program
    This class goes through header according to execution order
    */
    explicit DoStaticEvaluation(P4::ReferenceMap *refMap, P4::TypeMap *typeMap,
                                std::vector<ValueMapList*> *hdr_vec) :
            typeMap(typeMap), refMap(refMap), hdr_vec_list(hdr_vec) {
                    visitDagOnce = false;
                    factory = new P4::SymbolicValueFactory(typeMap);
                    setName("DoStaticEvaluation");}
    bool preorder(const IR::ToplevelBlock *tlb) override;
    bool preorder(const IR::P4Parser *block) override;
    bool preorder(const IR::SelectExpression *s) override;
    bool preorder(const IR::SelectCase *s) override;
    bool preorder(const IR::Path *path) override;
    bool preorder(const IR::P4Control *block) override;
    bool preorder(const IR::MethodCallStatement *stat) override;
    bool preorder(const IR::MethodCallExpression *expr) override;
    bool preorder(const IR::ParserState *state) override;
    bool preorder(const IR::P4Table *tab) override;
    bool preorder(const IR::ActionFunction *action) override;
    bool preorder(const IR::BlockStatement *block) override;
    bool preorder(const IR::IfStatement *stat) override;

    // postorder

    void postorder(const IR::BlockStatement *block) override;
    void postorder(const IR::SelectCase *s) override;
    void postorder(const IR::P4Control *block) override;
    void postorder(const IR::ParserState *s) override {
        LOG1_UNINDENT;
    };
    void postorder(const IR::SelectExpression *s) override;
};

class StaticEvaluation : public PassManager{
 public:
    explicit StaticEvaluation(P4::ReferenceMap *refMap, P4::TypeMap *typeMap,
                              std::vector<ValueMapList*> *hdr_vec)
            {
                auto evaluator = new P4::EvaluatorPass(refMap, typeMap);
                auto evaluation = new DoStaticEvaluation(refMap, typeMap, hdr_vec);
                passes.push_back(new P4::TypeChecking(refMap, typeMap));
                passes.push_back(evaluator);  // we visit from toplevel
                passes.push_back(new VisitFunctor([evaluator, evaluation](){
                    auto toplevel = evaluator->getToplevelBlock();
                    toplevel->apply(*evaluation);
                }));
                // passes.push_back(new DoStaticEvaluation(typeMap));
                setName("StaticEvaluation");
            }
};
}
#endif  // BACKENDS_P4FPGA_STATICEVAL_H_
