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
#include <cstdint>
#include <string>
#include <vector>
#include "backends/p4fpga/deparser.h"
#include "ir/indexed_vector.h"
#include "ir/ir-generated.h"
#include "ir/vector.h"
#include "frontends/p4/methodInstance.h"
#include "lib/cstring.h"
#include "lib/indent.h"
#include "lib/json.h"
#include "lib/log.h"
#include "lib/ordered_set.h"
#include "p4/methodInstance.h"
#include "p4fpga/JsonObjects.h"

namespace FPGA {

void DeparserConverter::insertTransition(){
    cstring label = "";
    if (condList->size() > 0){
        label = condList->at(condList->size() - 1);
        condList->pop_back();
    }
    if (previousState){
        LOG2("inserting links :" << IndentCtl::indent);
        for (auto ps : *previousState){
            for (auto cs : *currentState){
                auto res = links_set->insert(ps+cs+label);
                if (!res.second){
                    continue;
                }
                LOG2(ps << " -> " << cs);
                auto *transition = new Util::JsonObject();
                transition->emplace("source", ps);
                transition->emplace("target", cs);
                transition->emplace("label", label);
                links->append(transition);
            }
        }
        LOG2_UNINDENT;
    }
}

void DeparserConverter::insertState(state_t info){
    previousState = currentState;
    currentState = new ordered_set<cstring>();
    LOG1("inserting state " << info.name);
    cstring stateID = info.name + "_" + std::to_string(info.nbBits);
    stateID += "_" + std::to_string(info.startPos);
    state_set->insert(stateID);
    currentState->insert(stateID);
}

bool DeparserConverter::preorder(const IR::IfStatement* block){
    auto oriState = currentState;
    auto prev_emitBits = nbEmitBits;
    LOG2("visiting " << block->condition);
    condList->push_back(block->condition->toString());
    visit(block->ifTrue);
    nbEmitBits = prev_emitBits;
    if (block->ifFalse != nullptr){
        auto stateTrue = currentState;  // save state in True
        currentState = oriState;
        visit(block->ifFalse);
        nbEmitBits = prev_emitBits;
        // append state true to false
        for (auto cs : *stateTrue){
            currentState->insert(cs);
        }
    } else {
    // append state ori to state true
        for (auto cs : *oriState){
            currentState->insert(cs);
        }
    }
    // condList->pop_back();
    return false;
}

bool DeparserConverter::preorder(const IR::MethodCallStatement* s){
    // TODO add verification for extern type : emit statement
    auto mi = P4::MethodInstance::resolve(s->methodCall, refMap, typeMap);
    auto em = mi->to<P4::ExternMethod>();
    LOG2("visiting " << s);
    if (em->originalExternType->name.name == P4::P4CoreLibrary::instance.packetOut.name &&
        em->method->name.name == P4::P4CoreLibrary::instance.packetOut.emit.name) {
        auto mc = s->methodCall;
        auto arg = mc->arguments->at(0);
        auto hdrName = arg->toString();
        auto hdrW = uint64_t(typeMap->getType(arg)->width_bits());
        state_t stateParam = {hdrName, hdrW, nbEmitBits};
        nbEmitBits += hdrW;
        cstring stateName = hdrName + "_" + std::to_string(hdrW);
        stateName += "_" + std::to_string(nbEmitBits);
        stateName += "_" + std::to_string(nbEmitBits);
        insertState(stateParam);
        insertTransition();
    }
    return true;
}

bool DeparserConverter::preorder(const IR::P4Control* control) {
    links = new Util::JsonArray();
    links_set = new ordered_set<cstring>();
    state_set = new ordered_set<cstring>();
    condList = new std::vector<cstring>();
    nbEmitBits = 0;
    currentState = nullptr;
    state_t startState = {cstring("<start>"), 0, 0};
    insertState(startState);
    return true;
}

void DeparserConverter::postorder(const IR::P4Control* control) {
    Util::JsonObject* dep = new Util::JsonObject();
    dep->emplace("name", control->getName());
    Util::JsonArray*  state = new Util::JsonArray();
// insert end state
    state_t lastState = {cstring("<end>"), 0, nbEmitBits};
    insertState(lastState);
    insertTransition();
    for (auto i : *state_set){
        Util::JsonObject* tmp = new Util::JsonObject();
        tmp->emplace("id", i);
        state->append(tmp);
    }
    dep->emplace("nodes", state);
    dep->emplace("links", links);
    json->setDeparser(dep);
}

}  // namespace FPGA
