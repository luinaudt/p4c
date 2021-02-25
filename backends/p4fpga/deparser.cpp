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
Util::JsonObject* DeparserConverter::convertDeparser(const IR::P4Control* ctrl){
    Util::JsonObject* dep = new Util::JsonObject();
    dep->emplace("name", ctrl->getName());
    convertDeparserBody(&ctrl->body->components);
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
    return dep;
}

void DeparserConverter::insertTransition(){
    for(auto ps : *previousState){
        for(auto cs : *currentState){
            auto *transition = new Util::JsonObject();
            transition->emplace("source", ps);
            transition->emplace("target", cs);
            links->append(transition);
        }
    }   
}

bool DeparserConverter::preorder(const IR::IfStatement* block){
    auto prevState = currentState;
    auto condTrue = block->ifTrue->to<IR::BlockStatement>();
    if (condTrue) convertDeparserBody(&condTrue->components);
    else if (block->ifTrue->is<IR::StatOrDecl>()) convertStatement(block->ifTrue);
    auto lastState = currentState;
    currentState = prevState;
    auto condFalse = block->ifFalse->to<IR::BlockStatement>();
    if (condFalse) {
        convertDeparserBody(&condFalse->components);
        for(auto cs : *currentState){
            lastState->push_back(cs);
        }
    }
    else if (block->ifFalse->is<IR::StatOrDecl>()){
        convertStatement(block->ifFalse);
        for(auto cs : *currentState){
            lastState->push_back(cs);
        }
    }
    else{
        for(auto cs : *prevState){
            lastState->push_back(cs);
        }
    }
    currentState = lastState;
    return false;
}

void DeparserConverter::convertStatement(const IR::StatOrDecl* s){
    if(s->is<IR::MethodCallStatement>()){
        auto mc = s->to<IR::MethodCallStatement>()->methodCall;
        std::cout << "method call : " << mc << std::endl;
        auto arg = mc->arguments->at(0);
        state_set->insert(arg->toString());
        previousState = currentState;
        currentState = new std::vector<cstring>;
        currentState->push_back(arg->toString());
        insertTransition();
    }
    else if (s->is<IR::IfStatement>()) {
        auto cond = s->to<IR::IfStatement>();
        visit(cond);
    }
    else{
        std::cout <<"in convert statement " << s << std::endl;
        state_set->insert("else " + s->toString()); // DEVHELP - for info of unprocessed nodes
    }
}

void DeparserConverter::convertDeparserBody(const IR::Vector<IR::StatOrDecl>* body){
    std::cout << "current block : " << body << std::endl;
    for (auto s : *body) {
        if (auto block = s->to<IR::BlockStatement>()) {
            convertDeparserBody(&block->components);
            continue;
        }
        if(s->is<IR::IfStatement>()){
            auto cond = s->to<IR::IfStatement>();
            visit(cond);
        }
        else if(s->is<IR::StatOrDecl>()){
            std::cout << "state in convert deparser body : " << s << std::endl;
            convertStatement(s);
        }
    }
}

bool DeparserConverter::preorder(const IR::P4Control* control) {
    links = new Util::JsonArray();
    state_set = new ordered_set<cstring>();
    previousState = new std::vector<cstring>();
    auto startState = cstring("<start>");
    state_set->insert(startState);
    currentState = new std::vector<cstring>();
    currentState->push_back(startState);    
    auto deparserJson = convertDeparser(control);
    json->setDeparser(deparserJson);
    return false;
}
}