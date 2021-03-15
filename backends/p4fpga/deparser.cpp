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
#include "lib/cstring.h"
#include "lib/json.h"
#include "lib/ordered_set.h"
#include "p4/methodInstance.h"
#include "p4fpga/JsonObjects.h"
#include <iostream>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

namespace FPGA {

void DeparserConverter::insertTransition(){
    insertTransition("");
}
void DeparserConverter::insertTransition(cstring cond){
    cstring label = "";
    if(condList->size() > 0){
        for(auto c : *condList){
            label += c + " ";
        }
    }
    for(auto ps : *previousState){
        for(auto cs : *currentState){
            auto *transition = new Util::JsonObject();
            transition->emplace("source", ps);
            transition->emplace("target", cs);
            transition->emplace("label", label);
            links->append(transition);
        }
    }   
}

bool DeparserConverter::preorder(const IR::IfStatement* block){
    auto prevState = currentState;
    condList->push_back(block->condition->toString());
    visit(block->ifTrue);
    if (block->ifFalse != nullptr){
        condList->pop_back();
        condList->push_back("!" + block->condition->toString());
        auto lastState = currentState;
        currentState = prevState;
        visit(block->ifFalse);
        for(auto cs : *lastState){
            currentState->push_back(cs);
        }
    }
    for(auto cs : *prevState){
        currentState->push_back(cs);
    }
    condList->pop_back();
    return true;
}
bool DeparserConverter::preorder(const IR::MethodCallStatement* s){
    auto mc = s->methodCall;
    auto arg = mc->arguments->at(0);
    state_set->insert(arg->toString());
    previousState = currentState;
    currentState = new std::vector<cstring>;
    currentState->push_back(arg->toString());
    insertTransition();
    return true;
}

bool DeparserConverter::preorder(const IR::StatOrDecl* s){
    insertTransition();
    return true;
}


bool DeparserConverter::preorder(const IR::P4Control* control) {
    links = new Util::JsonArray();
    state_set = new ordered_set<cstring>();
    previousState = new std::vector<cstring>();
    condList = new std::vector<cstring>();
    auto startState = cstring("<start>");
    state_set->insert(startState);
    currentState = new std::vector<cstring>();
    currentState->push_back(startState);    
    return true;
}

void DeparserConverter::postorder(const IR::P4Control* control) {
    Util::JsonObject* dep = new Util::JsonObject();
    dep->emplace("name", control->getName());
    Util::JsonArray*  state = new Util::JsonArray();
// insert end state
    previousState = currentState;
    currentState = new std::vector<cstring>();
    currentState->push_back("<end>");
    insertTransition(); 
    state_set->insert("<end>");
    for(auto i : *state_set){
        Util::JsonObject* tmp = new Util::JsonObject();
        tmp->emplace("id", i);
        state->append(tmp);
    }
    dep->emplace("nodes", state);
    dep->emplace("links", links);
    json->setDeparser(dep);
}
}