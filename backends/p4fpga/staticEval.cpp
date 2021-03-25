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
#include "ir/ir.h"
#include "ir/node.h"
#include "lib/error.h"
#include "lib/exceptions.h"
#include "lib/indent.h"
#include "lib/log.h"
#include "lib/ordered_map.h"
#include "midend/interpreter.h"
#include "p4/methodInstance.h"
#include <iostream>
#include <stack>

namespace FPGA {

bool DoStaticEvaluation::preorder(const IR::ToplevelBlock *tlb) {
  LOG1("visiting program according to execution order");
  hdr_vec = new std::vector<P4::ValueMap*>();
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
  //while (!hdr_stack->empty()) {
    for(auto j : *hdr_vec){
      for(auto i : j->map){
          LOG1(i.first << 
                " of " << i.second);
      }
      //hdr_stack->pop();
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
    hdr = new P4::ValueMap();
    evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
    auto val = factory->create(paramType, true);
    hdr->set(hdrIn, val);
    auto pktIn = block->getApplyParameters()->parameters.at(0);
    val = factory->create(typeMap->getType(pktIn), false);
    hdr->set(pktIn, val);
    LOG1(val);
    return true;
}
void DoStaticEvaluation::postorder(const IR::P4Parser *block){
    hdr_vec->push_back(hdr);
    LOG1_UNINDENT;
};

bool DoStaticEvaluation::preorder(const IR::ParserState *s){
    LOG1("visiting parser state " << s->name << IndentCtl::indent);
    return true;
}
bool DoStaticEvaluation::preorder(const IR::SelectCase *s){
    LOG2(s->static_type_name() << " "<< s);
    auto etat = s->state;
    visit(refMap->getDeclaration(etat->path)->to<IR::Node>());
    return true;
}
void DoStaticEvaluation::postorder(const IR::SelectCase *s){
    LOG2("postorder " << s->static_type_name() << " "<< s);
}

bool DoStaticEvaluation::preorder(const IR::P4Control *block){
    LOG1("visiting " << block->static_type_name() << " " << block->getName() << IndentCtl::indent);
    auto hdrIn = block->getApplyParameters()->parameters.at(0);
    auto paramType = typeMap->getType(hdrIn);
    if(!paramType->is<IR::Type_Struct>()){
        //FIXME - Special case for deparser 
        hdrIn = block->getApplyParameters()->parameters.at(1);
        paramType = typeMap->getType(hdrIn);
    }
    if(!paramType->is<IR::Type_Struct>()){
        ::error(ErrorType::ERR_UNEXPECTED,
                "%1%: param is not a struct", paramType);
    }
    auto newMap = new P4::ValueMap();
    newMap->set(hdrIn, hdr->map.begin()->second);
    hdr = newMap;
    evaluator = new P4::ExpressionEvaluator(refMap, typeMap, hdr);
    return true;
}

bool DoStaticEvaluation::preorder(const IR::MethodCallStatement *stat){
    LOG2(stat->static_type_name() << "  "<< stat->toString());
    auto mi = P4::MethodInstance::resolve(stat->methodCall, refMap, typeMap);
    if(auto bim = mi->to<P4::BuiltInMethod>()){
        LOG2(bim);
    }
    if(mi->is<P4::ExternMethod>()){
        // avoid evaluation of packet.extract
        return true;
    }
    return true;
}
bool DoStaticEvaluation::preorder(const IR::MethodCallExpression *expr){
    LOG2(expr->static_type_name() << "  "<< expr->toString());
    auto res = evaluator->evaluate(expr, false);
    LOG2(res);
    return true;
}

} // namespace FPGA