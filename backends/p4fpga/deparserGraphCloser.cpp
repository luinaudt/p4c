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
#include <vector>
#include "lib/indent.h"
#include "lib/log.h"
#include "backends/p4fpga/deparserGraphCloser.h"
#include "lib/exceptions.h"
#include "midend/interpreter.h"

namespace FPGA {

const IR::Node* doDeparserGraphCloser::preorder(IR::P4Program* prog) {
    LOG1("closing graph " << IndentCtl::indent);
    // we extract the name of the deparser
    auto mainDecls = prog->getDeclsByName(IR::P4Program::main)->toVector();
    auto main = mainDecls->at(0)->to<IR::Declaration_Instance>();
    auto deparser = main->arguments->at(main->arguments->size()-1);  // Deparser is last 
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

const IR::Node* doDeparserGraphCloser::preorder(IR::P4Control* ctrl){
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
const IR::Node* doDeparserGraphCloser::postorder(IR::P4Control* ctrl){
    LOG1_UNINDENT;
    LOG1("closing graph postorder");
    return ctrl;
}

const IR::Node* doDeparserGraphCloser::preorder(IR::BlockStatement* block){
    LOG1("in blockStatement " << block);
    return block;
}
const IR::Node* doDeparserGraphCloser::preorder(IR::IfStatement* cond){
    // TODO
    // P4C_UNIMPLEMENTED("if statement in deparser");
    LOG1("preorder " << cond->static_type_name());
    LOG1(cond);
    LOG2("evaluate " << cond->condition);
    auto val = P4::SymbolicBool();
    for (auto hdr : *hdr_vec) {
        LOG3(" with " << hdr);
        evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
        auto res = evaluator->evaluate(cond->condition , false);
        BUG_CHECK(res->is<P4::SymbolicBool>(), "%1% condition error", cond->condition);
        auto condRes = res->to<P4::SymbolicBool>();
        if (val.isUninitialized()){
            val.value = condRes->value;
            val.state=P4::ScalarValue::ValueState::Constant;
        } else if (val.value!=condRes->value) {
            val.state=P4::ScalarValue::ValueState::NotConstant;
            break;
        }
    }
    // constant value
    if (val.isKnown()){
        LOG1("valKnown ");
        // auto stats = new IR::IndexedVector<IR::StatOrDecl>();
        if (val.value){
            LOG1("returning : " << cond->ifTrue);
            return cond->ifTrue;
        } else {
            LOG1("returning : " << cond->ifFalse);
            return cond->ifFalse;
        }
    }
    LOG1("returning : " << cond);
    return cond;
}

const IR::Node* doDeparserGraphCloser::postorder(IR::IfStatement* cond){
    // TODO
    // P4C_UNIMPLEMENTED("if statement in deparser");
    LOG1("postorder " << cond->static_type_name());
    LOG1(cond);
    return cond;
}

const IR::Node* doDeparserGraphCloser::preorder(IR::StatOrDecl* s){
    LOG1("in state or decl" << s);
    if (s->is<IR::MethodCallStatement>()){
        auto mc = s->to<IR::MethodCallStatement>()->methodCall;
    }else if (s->is<IR::IfStatement>()) {
        auto cond = s->to<IR::IfStatement>();
        visit(cond);
    }
    return s;
}

const IR::BlockStatement* doDeparserGraphCloser::convertBody(const IR::Vector<IR::StatOrDecl>* body){
    for (auto s : *body) {
        if (auto block = s->to<IR::BlockStatement>()) {
            convertBody(&block->components);
            continue;
        }
        if (s->is<IR::IfStatement>()){
            auto cond = s->to<IR::IfStatement>();
            visit(cond);
        }else if (s->is<IR::StatOrDecl>()){
            auto statement = s->to<IR::StatOrDecl>();
            visit(statement);
        }
    }
    auto newBody = new IR::BlockStatement();
    return newBody;
}

}  // namespace FPGA
