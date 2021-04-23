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

#include <cstdlib>
#include <iostream>
#include <vector>
#include "lib/indent.h"
#include "lib/log.h"
#include "backends/p4fpga/reachabilitySimplifier.h"
#include "lib/exceptions.h"
#include "midend/interpreter.h"

namespace FPGA {

const IR::Node* doReachabilitySimplifier::preorder(IR::P4Program* prog) {
    LOG1("Simplifying graph " << IndentCtl::indent);
    // we extract the name of the deparser
    auto mainDecls = prog->getDeclsByName(IR::P4Program::main)->toVector();
    auto main = mainDecls->at(0)->to<IR::Declaration_Instance>();
    // Deparser is last
    auto deparser = main->arguments->at(main->arguments->size()-1);
    auto depType = deparser->expression->type->getP4Type();
    int pos = 0;
    for (auto i : prog->objects) {
        if (i->is<IR::P4Control>()){
            auto block = i->to<IR::P4Control>();
            // we only visit deparsers.
            if (block->type->getP4Type()->equiv(*depType->getNode())) {
                LOG1(block);
                visit(block);
                prog->objects.at(pos) = block;
            }
        }
        pos++;
    }
    prune();
    LOG1_UNINDENT;
    return prog;
}

const IR::Node* doReachabilitySimplifier::preorder(IR::P4Control* ctrl){
    auto newHdr_vec = new std::vector<P4::ValueMap*>();
    auto hdrIn = ctrl->getApplyParameters()->parameters.at(1);
    auto paramType = typeMap->getType(hdrIn);
    if (!paramType->is<IR::Type_Struct>()){
        ::error(ErrorType::ERR_UNEXPECTED,
                "%1%: param is not a struct", paramType);}
    // update values Map with correct references
    auto newMap = new P4::ValueMap();
    for (auto hdr : *hdr_vec){
        newMap->set(hdrIn, hdr->map.begin()->second);
        newHdr_vec->push_back(newMap->clone());
    }
    hdr_vec = newHdr_vec;  // assign update list
    return ctrl;
}
const IR::Node* doReachabilitySimplifier::postorder(IR::BlockStatement* block){
    if (block == nullptr ||
        block->components.size()==0) {
            return nullptr;
    }
    if (block->components.size()==1){
        return block->components.at(0);
    }
    return block;
}
const IR::Node* doReachabilitySimplifier::preorder(IR::IfStatement* cond){
    // TODO
    // P4C_UNIMPLEMENTED("if statement in deparser");
    LOG1("preorder " << cond->static_type_name() << IndentCtl::indent);
    LOG1(cond);
    if (hdr_vec->size()==0){
        LOG2("no possible value return nullptr");
        return nullptr;
    }
    LOG2("evaluate " << cond->condition);
    auto hdr_val_true = new std::vector<P4::ValueMap*> ();
    auto hdr_val_false = new std::vector<P4::ValueMap*> ();
    // evaluate for all possible condition value
    for (auto hdr : *hdr_vec) {
        LOG3(" with " << hdr);
        evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
        auto res = evaluator->evaluate(cond->condition , false);
        BUG_CHECK(res->is<P4::SymbolicBool>(), "%1% condition error", cond->condition);
        auto condRes = res->to<P4::SymbolicBool>();
        if (condRes->value){
            hdr_val_true->push_back(hdr);
        } else{
            hdr_val_false->push_back(hdr);
        }
    }
    // visit condition inside with assign possible header validity
    // set condition modification (code simplification)
    // constant value
    auto old_hdr_vec = hdr_vec;
    if (hdr_val_true->size() != 0) {
        hdr_vec = hdr_val_true;
        LOG3("visitCondTrue");
        visit(cond->ifTrue);
    }
    if (hdr_val_false->size() != 0){
        hdr_vec = hdr_val_false;
        LOG3("visitCondFalse");
        visit(cond->ifFalse);
    }
    LOG1_UNINDENT;
    hdr_vec = old_hdr_vec;
    prune();
    if (hdr_val_false->size() == 0) {
        return cond->ifTrue->clone();
    } else if (hdr_val_true->size() == 0) {
        if (cond->ifFalse != nullptr) {
            return cond->ifFalse->clone();
        }
        return nullptr;
    }else{
        return cond->clone();
    }
}
}  // namespace FPGA
