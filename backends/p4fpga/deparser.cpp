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
#include "backends/p4fpga/deparser.h"
#include "ir/indexed_vector.h"
#include "ir/ir-generated.h"
#include "ir/vector.h"
#include "lib/json.h"
#include "lib/ordered_set.h"
#include "p4fpga/JsonObjects.h"
#include <iostream>
#include <ostream>
#include <string>

namespace FPGA {
Util::JsonObject* DeparserConverter::convertDeparser(const IR::P4Control* ctrl){
    Util::JsonObject* dep = new Util::JsonObject();
    dep->emplace("name", ctrl->getName());
    convertDeparserBody(&ctrl->body->components);
    Util::JsonArray*  state = new Util::JsonArray();
    state_set->insert("end");
    for(auto i : *state_set){
        state->append(i);
    }
    dep->emplace("nodes", state);
    dep->emplace("links", links);
    return dep;
}

void DeparserConverter::convertStatement(const IR::StatOrDecl* s){
    if(s->is<IR::MethodCallStatement>()){
        auto mc = s->to<IR::MethodCallStatement>()->methodCall;
        std::cout << "method call : " << mc << std::endl;
        auto arg = mc->arguments->at(0);
        state_set->insert(arg->toString());
    }
    else state_set->insert("else " + s->toString()); // DEVHELP - for info of unprocessed nodes
}

void DeparserConverter::convertDeparserBody(const IR::Vector<IR::StatOrDecl>* body){
    std::cout << "current block : " << body << std::endl;
    for (auto s : *body) {
        if (auto block = s->to<IR::BlockStatement>()) {
            convertDeparserBody(&block->components);
            continue;
        }
        if(s->is<IR::IfStatement>()){
            auto block = s->to<IR::IfStatement>();
            auto condTrue = block->ifTrue->to<IR::BlockStatement>();
            if (condTrue) convertDeparserBody(&condTrue->components);
            else if (block->ifTrue->is<IR::StatOrDecl>()) convertStatement(block->ifTrue);
            auto condFalse = block->ifFalse->to<IR::BlockStatement>();
            if (condFalse) convertDeparserBody(&condFalse->components);
            else if (block->ifFalse->is<IR::StatOrDecl>()) convertStatement(block->ifFalse);
        }
        else if(s->is<IR::StatOrDecl>()) convertStatement(s);
        else state_set->insert("else " + s->toString()); // DEVHELP - for info of unprocessed nodes       
    }
}

bool DeparserConverter::preorder(const IR::P4Control* control) {
    links = new Util::JsonArray();
    state_set = new ordered_set<cstring>();
    state_set->insert("start");
    auto deparserJson = convertDeparser(control);
    json->setDeparser(deparserJson);
    return false;
}
}