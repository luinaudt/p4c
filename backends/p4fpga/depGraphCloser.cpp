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

#include "backends/p4fpga/depGraphCloser.h"
#include "ir/node.h"
#include "lib/exceptions.h"
#include <iostream>

namespace FPGA {
    
     const IR::Node* DeparserGraphCloser::postorder(const IR::P4Control* ctrl){
        convertBody(&ctrl->body->components);
        return nullptr;
    }
    bool DeparserGraphCloser::preorder(const IR::IfStatement* cond){
        //TODO 
        P4C_UNIMPLEMENTED("if statement in deparser");
        return false;
    }
    bool DeparserGraphCloser::preorder(const IR::StatOrDecl* s){
        if(s->is<IR::MethodCallStatement>()){
            auto mc = s->to<IR::MethodCallStatement>()->methodCall;
        }
        else if (s->is<IR::IfStatement>()) {
            auto cond = s->to<IR::IfStatement>();
            visit(cond);
        }
        return false;
    }
    const IR::Node* DeparserGraphCloser::convertBody(const IR::Vector<IR::StatOrDecl>* body){; 
    for (auto s : *body) {
        if (auto block = s->to<IR::BlockStatement>()) {
            convertBody(&block->components);
            continue;
        }
        if(s->is<IR::IfStatement>()){
            auto cond = s->to<IR::IfStatement>();
            visit(cond);
        }
        else if(s->is<IR::StatOrDecl>()){
            auto statement = s->to<IR::StatOrDecl>();
            visit(statement);
        }
    }
    return nullptr;
}

}