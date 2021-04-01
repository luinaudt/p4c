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


#include "backends/p4fpga/staticEval.h"
#include "ir/ir-generated.h"
#include "ir/ir.h"
#include "ir/node.h"
#include "lib/error.h"
#include "lib/error_catalog.h"
#include "lib/exceptions.h"
#include "lib/indent.h"
#include "lib/log.h"
#include "lib/ordered_map.h"
#include "midend/interpreter.h"
#include "p4/methodInstance.h"
#include <stack>

namespace FPGA {

bool DoStaticEvaluation::preorder(const IR::ToplevelBlock *tlb) {
    LOG1("visiting program according to execution order");
    hdr_vec = new std::vector<P4::ValueMap*>();
    hdr = new P4::ValueMap();
    auto main = tlb->getMain();
    auto param = main->getConstructorParameters();
    for (auto i : *param) {
        auto arg = main->getParameterValue(i->getName());
        if (arg->is<IR::ParserBlock>()) {
            auto block = arg->to<IR::ParserBlock>()->container;
            visit(block);
        } else if (arg->is<IR::ControlBlock>()) {
            auto block = arg->to<IR::ControlBlock>()->container;
            visit(block);
        } else {
            LOG1("other " << arg->toString());
        }
    }
    auto tmpHdr = hdr->map.begin()->second->clone()->to<P4::SymbolicStruct>();
    tmpHdr->setAllUnknown();
    for(auto j : *hdr_vec){
        for(auto i : j->map){
            if(i.second->is<P4::SymbolicStruct>()){
                auto hdr_elems = i.second->to<P4::SymbolicStruct>();
                for(auto he : hdr_elems->fieldValue){
                    LOG3("analyzing " << he.first);
                    if(he.second->is<P4::SymbolicHeader>() &&
                        he.second->to<P4::SymbolicHeader>()->valid->value){
                        tmpHdr->set(he.first, he.second);
                    }   
                }
            }
            LOG1(i.first << 
                " of " << i.second);
        } 
    }
    for(auto i: tmpHdr->fieldValue){
        LOG3("checking usage of " << i.first);
        if(i.second->is<P4::SymbolicHeader>()){
            auto h = i.second->to<P4::SymbolicHeader>();
            if(h->valid->isUnknown() || !h->valid->value){
                LOG3("validity : " << h->valid);
                ::warning(ErrorType::WARN_UNUSED, "header %1% is not used", i.first);
            }
        }
    }
  return true;
}

bool DoStaticEvaluation::preorder(const IR::P4Parser *block){
    LOG1("visiting " << block->static_type_name() << " " << block->getName() << IndentCtl::indent);
    auto hdrIn = block->getApplyParameters()->parameters.at(1);
    auto paramType = typeMap->getType(hdrIn);
    if(!paramType->is<IR::Type_Struct>()){
        ::error(ErrorType::ERR_UNEXPECTED,
                "%1%: param is not a struct", paramType);
    }
    evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
    auto val = factory->create(paramType, true);
    hdr->set(hdrIn, val);
    auto pktIn = block->getApplyParameters()->parameters.at(0);
    val = factory->create(typeMap->getType(pktIn), false);
    hdr->set(pktIn, val);
    // we start by visiting start state, ignore any unfollowed states
    for(auto s : block->states){
        if(s->name.name == IR::ParserState::start) {
            visit(s);
            LOG1_UNINDENT;
            return false;
        }
    }
    BUG("%1% start state not found", block);
    return false;
}

bool DoStaticEvaluation::preorder(const IR::ParserState *s){
    LOG1("visiting " << s->static_type_name() << " " << s->name << IndentCtl::indent);
    return true;
}



bool DoStaticEvaluation::preorder(const IR::SelectCase *s){
    LOG2(s->static_type_name() << " "<< s);
    auto etat = s->state;
   
    auto next = refMap->getDeclaration(etat->path)->to<IR::Node>();
    if(next == nullptr){
        ::error(ErrorType::ERR_UNREACHABLE, "state %1%", s);
    }
    LOG2("next Select state is : " << etat->path->name);
    //visit(etat);
    return true;
}

void DoStaticEvaluation::postorder(const IR::SelectCase *s){
    LOG2("postorder " << s->static_type_name() << " "<< s);
    BUG_CHECK(!hdr_stack.empty(), "%1%: empty hdr stack", s);
    hdr = hdr_stack.top()->clone();
    evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
}

bool DoStaticEvaluation::preorder(const IR::SelectExpression *s){
    LOG2(s->static_type_name() << " "<< s);
    hdr_stack.push(hdr->clone());
    return true;
}

void DoStaticEvaluation::postorder(const IR::SelectExpression *s){
    LOG2("postorder " << s->static_type_name() << " "<< s);
    BUG_CHECK(!hdr_stack.empty(), "%1%: empty hdr stack", s);
    hdr = hdr_stack.top()->clone();
    evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
    hdr_stack.pop();
}

void DoStaticEvaluation::update_hdr_vec(P4::ValueMap* val){
    static P4::ValueMap* prev = nullptr; //saving previous find, could be a list
    bool in = prev!=nullptr && prev->equals(val); //possibly in
    if(!in){ //possibly not
        //It is highly probable that last inserted will be equal to current
        for(auto i= hdr_vec->crbegin(); i!=hdr_vec->crend(); ++i){
            if(val->equals(*i)){
                in=true;
                prev=*i;
                break;
            }
        }
    }
    if(!in){
        hdr_vec->push_back(val);
        prev=val;
        LOG2("pushing valid headers : " << val);
    } 
    else{ 
        LOG3("valid headers already exists : " << val); 
    }
}
bool DoStaticEvaluation::preorder(const IR::Path *path){
    LOG3("visiting " << path->static_type_name() << " "<< path);
    if(path->name == IR::ParserState::accept || 
            path->name == IR::ParserState::reject){
        LOG2("terminal state");
        update_hdr_vec(hdr->clone());
        return true;
    }
    auto next=refMap->getDeclaration(path, false)->to<IR::Node>();
    if(next->is<IR::ParserState>()){
        LOG2("next state is : " << path->name);
        visit(next);
    }
    return true;
}
bool DoStaticEvaluation::preorder(const IR::P4Control *block){
    LOG1("visiting " << block->static_type_name() << " " << block->getName() << IndentCtl::indent);
    auto hdrIn = block->getApplyParameters()->parameters.at(0);
    auto paramType = typeMap->getType(hdrIn);
    bool isDep = false;
    if(!paramType->is<IR::Type_Struct>()){
        //FIXME - Special case for deparser 
        hdrIn = block->getApplyParameters()->parameters.at(1);
        paramType = typeMap->getType(hdrIn);
        isDep = true;
    }
    if(!paramType->is<IR::Type_Struct>()){
        ::error(ErrorType::ERR_UNEXPECTED,
                "%1%: param is not a struct", paramType);
    }
    auto newMap = new P4::ValueMap();
    newMap->set(hdrIn, hdr->map.begin()->second);
    hdr = newMap;
    evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
    // FIXME - for test
    if (isDep){
        return true;
    }
    LOG1_UNINDENT;
    return isDep;
}

bool DoStaticEvaluation::preorder(const IR::MethodCallStatement *stat){
    auto mi = P4::MethodInstance::resolve(stat->methodCall, refMap, typeMap);
    if(auto bim = mi->to<P4::BuiltInMethod>()){
        LOG2(stat->static_type_name() << "  "<< bim->appliedTo << bim->name);
        return true;
    }
    return false;
}
bool DoStaticEvaluation::preorder(const IR::MethodCallExpression *expr){
    
    auto res = evaluator->evaluate(expr, false);
    LOG2("evaluation of " << expr->toString());
    LOG3("  got: " << res);
    return true;
}

} // namespace FPGA