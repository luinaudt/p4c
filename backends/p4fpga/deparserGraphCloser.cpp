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

const IR::Node* doDeparserGraphCloser::preorder(IR::P4Control* ctrl){
    LOG1("in control " << ctrl);
    futures = new IR::IndexedVector<IR::StatOrDecl>();
    visit(ctrl->body);
    prune();
    return ctrl;
}

const IR::Node* doDeparserGraphCloser::preorder(IR::BlockStatement* block){
    LOG1("in blockStatement " << block);
    for (auto inst =  block->components.rbegin();
         inst != block->components.rend(); inst++){
        visit(*inst);
    }
    prune();
    return new IR::BlockStatement(block->srcInfo, *futures);
}
const IR::Node* doDeparserGraphCloser::preorder(IR::IfStatement* cond){
    // TODO
    LOG1("preorder " << cond->static_type_name());
    LOG2(cond);
    auto oldFutures = futures->clone();
    auto newCond = cond->clone();
    // ifTrue transformation
    visit(cond->ifTrue);
    if (futures->size() != 0){
        newCond->ifTrue = new IR::BlockStatement(*futures);
    }
    // ifFalse transformation
    futures = oldFutures->clone();
    visit(cond->ifFalse);
    if (futures->size() != 0){
        newCond->ifFalse = new IR::BlockStatement(*futures);
    }
    // update futures with condition
    futures = new IR::IndexedVector<IR::StatOrDecl>();
    futures->insert(futures->begin(), newCond->clone());
    prune();
    return newCond;
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
    futures->insert(futures->begin(), s->clone());
    return s;
}

}  // namespace FPGA
