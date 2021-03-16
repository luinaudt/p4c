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
#include "frontends/p4/methodInstance.h"
#include "lib/cstring.h"
#include "lib/json.h"
#include "lib/log.h"
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
    if(previousState){
        for(auto ps : *previousState){
            for(auto cs : *currentState){
                LOG1("insert link " << ps << " to " << cs);
                auto *transition = new Util::JsonObject();
                transition->emplace("source", ps);
                transition->emplace("target", cs);
                transition->emplace("label", label);
                links->append(transition);
            }
        }
    }   
}

bool DeparserConverter::preorder(const IR::IfStatement* block){
    auto oriState = currentState;
    condList->push_back(block->condition->toString());
    visit(block->ifTrue);
    if (block->ifFalse != nullptr){
        condList->pop_back();
        condList->push_back("!" + block->condition->toString());
        auto stateTrue = currentState; // save state in True
        currentState = oriState;
        visit(block->ifFalse);
        // append state true to false
        for(auto cs : *stateTrue){
            currentState->insert(cs);
        }
    }
    // append state ori to state true
    for(auto cs : *oriState){
        currentState->insert(cs);
    }
    condList->pop_back();
    return true;
}
bool DeparserConverter::preorder(const IR::MethodCallStatement* s){
    // TODO add verification for extern type : emit statement
    auto mi = P4::MethodInstance::resolve(s->methodCall, refMap, typeMap);
    auto em = mi->to<P4::ExternMethod>();
    if (em->originalExternType->name.name == P4::P4CoreLibrary::instance.packetOut.name &&
        em->method->name.name == P4::P4CoreLibrary::instance.packetOut.emit.name) {
        auto mc = s->methodCall;
        auto arg = mc->arguments->at(0);
        auto hdrName = arg->toString();
        auto hdrW = typeMap->getType(arg)->width_bits();
        cstring stateName = hdrName + "_" + std::to_string(hdrW);
        state_set->insert(stateName);
        previousState = currentState;
        currentState = new ordered_set<cstring>;
        currentState->insert(stateName);

        insertTransition();
        LOG1("emitting " << hdrName << " width " << hdrW << "bits");
    }
    return true;
}

bool DeparserConverter::preorder(const IR::P4Control* control) {
    links = new Util::JsonArray();
    state_set = new ordered_set<cstring>();
    previousState = nullptr;
    condList = new std::vector<cstring>();
    auto startState = cstring("<start>");
    state_set->insert(startState);
    currentState = new ordered_set<cstring>();
    currentState->insert(startState);    
    return true;
}

void DeparserConverter::postorder(const IR::P4Control* control) {
    Util::JsonObject* dep = new Util::JsonObject();
    dep->emplace("name", control->getName());
    Util::JsonArray*  state = new Util::JsonArray();
// insert end state
    previousState = currentState;
    currentState = new ordered_set<cstring>();
    currentState->insert("<end>");
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