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
#include <utility>
#include <vector>
#include "backends/p4fpga/deparser.h"
#include "ir/indexed_vector.h"
#include "ir/ir-generated.h"
#include "ir/vector.h"
#include "frontends/p4/methodInstance.h"
#include "lib/cstring.h"
#include "lib/error.h"
#include "lib/error_catalog.h"
#include "lib/indent.h"
#include "lib/json.h"
#include "lib/log.h"
#include "lib/null.h"
#include "lib/ordered_set.h"
#include "p4/methodInstance.h"
#include "p4fpga/JsonObjects.h"

namespace FPGA {

// state
cstring emitState::getStateName(){
    cstring name = "";
    if (bitMap.size() == 0){
        return name;
    }
    name += bitMap[0]->first+"_";
    name += std::to_string(bitMap[0]->second.first);
    name += "_" + std::to_string(bitMap[0]->second.second);
    for (uint32_t i = 1; i < bitMap.size(); ++i) {
        name += "+";
        name += bitMap[i]->first + "_";
        name += std::to_string(bitMap[i]->second.first);
        name += "_" + std::to_string(bitMap[i]->second.second);
    }
    return name;
}

emitState* emitState::clone(){
    auto newEmit = new emitState(width);
    newEmit->bitMap = std::vector<bitInfo_t*>(bitMap);
    newEmit->pos = pos;
    return newEmit;
}
int64_t emitState::insertHdr(cstring hdrName, uint64_t posInHdr, uint64_t nbBitsToInsert){
    uint64_t insertBits = width - pos;
    if (nbBitsToInsert < insertBits) {
        insertBits = nbBitsToInsert;
    }
    bitInfo_t* newHdr = new bitInfo_t(hdrName, bitPosLen_t(posInHdr, insertBits));
    bitMap.push_back(newHdr);
    pos += insertBits;
    return (pos == width) ? nbBitsToInsert - insertBits : pos - width;
}


// deparser converter
void DeparserConverter::insertTransition(){
    cstring label = std::to_string(priority);
    cstring cond = "";
    if (condList->size() > 0){
        label += " : ";
        for (auto i : *condList) {
            cond += i;
        }
        label += cond;
        condList->pop_back();
        condList = new std::vector<cstring>();
    }
    if (previousState){
        LOG2("inserting links :" << IndentCtl::indent);
        for (auto ps : *previousState){
            for (auto cs : *currentState){
                auto res = links_set->insert(ps+cs+cond);
                if (!res.second){
                    continue;
                }
                LOG2(ps << " -> " << cs << " " << cond);
                auto *transition = new Util::JsonObject();
                transition->emplace("source", ps);
                transition->emplace("target", cs);
                transition->emplace("label", label);
                transition->emplace("priority", priority);
                (cond != "") ? transition->emplace("condition", cond) : nullptr;
                links->append(transition);
            }
        }
        LOG2_UNINDENT;
    }
}
void DeparserConverter::insertState(cstring info){
    previousState = currentState;
    currentState = new ordered_set<cstring>();
    LOG1("inserting state " << info);
    if (state_set->insert(info).second){
        Util::JsonObject* tmp = new Util::JsonObject();
        tmp->emplace("id", info);
        statesList->append(tmp);
    }
    currentState->insert(info);
}

void DeparserConverter::insertState(emitState* info){
    cstring stateID = info->getStateName();
    previousState = currentState;
    currentState = new ordered_set<cstring>();
    LOG1("inserting state " << stateID);
    if (state_set->insert(stateID).second){
        Util::JsonObject* stateObject = new Util::JsonObject();
        stateObject->emplace("id", stateID);
        Util::JsonArray* elem = new Util::JsonArray();
        Util::JsonArray* pos = new Util::JsonArray();
        Util::JsonArray* nbBits = new Util::JsonArray();
        for (auto i : info->bitMap) {
            elem->append(i->first);
            pos->append(i->second.first);
            nbBits->append(i->second.second);
        }
        stateObject->emplace("HdrName", elem);
        stateObject->emplace("HdrPos", pos);
        stateObject->emplace("HdrLen", nbBits);
        statesList->append(stateObject);
    }
    currentState->insert(stateID);
}

bool DeparserConverter::preorder(const IR::IfStatement* block){
    auto oriEmitState = state->clone();
    auto oriState = currentState;
    auto prev_emitBits = nbEmitBits;
    LOG2("visiting " << block->condition);
    condList->push_back(block->condition->toString());
    visit(block->ifTrue);
    nbEmitBits = prev_emitBits;
    state = oriEmitState;
    if (block->ifFalse != nullptr){
        priority++;
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
    return false;
}

cstring DeparserConverter::headerName(const IR::Member *expression){
    cstring argName = "";
    CHECK_NULL(expression);
    auto type = typeMap->getType(expression->expr);
    if (type->is<IR::Type_StructLike>()){
        argName = expression->member;
    }
    return argName;
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
        auto hdrName = headerName(arg->expression->to<IR::Member>());
        auto hdrW = uint32_t(typeMap->getType(arg)->width_bits());
        LOG1("emitting " << hdrW << " bits for hdr " << hdrName);
        // inserting for all possible positions
        if (splitStates==false) {
            state = new emitState(hdrW);
        }
        int64_t remain = hdrW;
        remain = state->insertHdr(hdrName, 0, remain);
        uint64_t hdrPos = hdrW - remain;  // move from remaining bits
        while (remain > 0) {
            insertState(state);
            insertTransition();
            state = new emitState(outputBusWidth);
            remain = state->insertHdr(hdrName, hdrPos, remain);
            hdrPos += outputBusWidth;
        };
        if (remain == 0){
            insertState(state);
            insertTransition();
            state = new emitState(outputBusWidth);
        }else{
            LOG1("remaining " << remain << " bits for hdr " << hdrName);
        }
    }
    return true;
}

bool DeparserConverter::preorder(const IR::P4Control* control) {
    // init deparser info (case object is reused)
    deparserJson = new Util::JsonObject();
    cstring startState = cstring("<start>");
    links = new Util::JsonArray();
    statesList = new Util::JsonArray();
    links_set = new ordered_set<cstring>();
    state_set = new ordered_set<cstring>();
    condList = new std::vector<cstring>();
    // json init
    Util::JsonObject* phv = new Util::JsonObject();
    deparserJson->emplace("name", control->getName());
    auto hdrIn = control->getApplyParameters()->parameters.at(1);
    auto hdrType = typeMap->getType(hdrIn);
    if (hdrType->is<IR::Type_Struct>()) {
        auto hdrs = hdrType->to<IR::Type_Struct>();
        for (auto i : hdrs->fields){
            auto ht=typeMap->getType(i);
            if (ht->is<IR::Type_Stack>()) {
                ::error(ErrorType::ERR_UNSUPPORTED,
                        "headers stack are not supported in deparser yet %1%",
                        i->toString());
            }
            phv->emplace(i->toString(), uint32_t(ht->width_bits()));
        }
    }
    deparserJson->emplace("PHV", phv);
    deparserJson->emplace("startState", startState);
    // state machine setupe
    currentState = nullptr;
    priority = 0;
    insertState(startState);
    return true;
}

void DeparserConverter::postorder(const IR::P4Control* control) {
    cstring endState = cstring("<end>");
    deparserJson->emplace("lastState", endState);
// insert end state
    priority++;
    insertState(endState);
    insertTransition();
    auto depGraph = new Util::JsonObject();
    depGraph->emplace("nodes", statesList);
    depGraph->emplace("links", links);
    deparserJson->emplace("graph", depGraph);
    json->setDeparser(deparserJson);
}

}  // namespace FPGA
