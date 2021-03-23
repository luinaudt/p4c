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
#include "lib/error.h"
#include "lib/exceptions.h"
#include "lib/indent.h"
#include "lib/log.h"
#include "lib/ordered_map.h"
#include "midend/interpreter.h"
#include <stack>

namespace FPGA {

bool DoStaticEvaluation::preorder(const IR::ToplevelBlock *tlb) {
  LOG1("visiting program according to execution order");
  hdr_vec = new std::vector<hdr_value*>();
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
      for(auto i : *j){
          LOG1("hdr " << i.first->name << 
          " of Type " << typeMap->getType(i.first)->to<IR::Type_Header>()->name <<
          " is " << ((i.second) ? "true" : "false"));
      }
      //hdr_stack->pop();
  }
  return true;
}

bool DoStaticEvaluation::preorder(const IR::P4Parser *block){
    LOG1("visiting " << block->static_type_name() << " " << block->getName() << IndentCtl::indent);
    auto param = typeMap->getType(block->getApplyParameters()->parameters.at(1));
    if(!param->is<IR::Type_Struct>()){
        ::error(ErrorType::ERR_UNEXPECTED,
                "%1%: param is not a struct", param);
    }
    hdr = new hdr_value();
    auto paramType = param->to<IR::Type_Struct>();
    for(auto i : paramType->fields){
        auto elem = typeMap->getType(i);
        if(elem->is<IR::Type_Header>()){
            hdr->emplace(i, false);
        }
        else{
            P4C_UNIMPLEMENTED("only header are supporter for static analysis");
        }
    }
    visit(block->states);
    return true;
}
bool DoStaticEvaluation::preorder(const IR::ParserState *s){
    LOG1("visiting parser state" << IndentCtl::indent);
    return true;
}
void DoStaticEvaluation::postorder(const IR::ParserState *s){
    LOG1_UNINDENT;
}

void DoStaticEvaluation::postorder(const IR::P4Parser *block){
    LOG1_UNINDENT;
}

bool DoStaticEvaluation::preorder(const IR::P4Control *block){
    LOG1("visiting " << block->static_type_name() << " " << block->getName() << IndentCtl::indent);
    return true;
}
void DoStaticEvaluation::postorder(const IR::P4Control *block){
    LOG1_UNINDENT;
}

bool DoStaticEvaluation::preorder(const IR::MethodCallStatement *stat){
    LOG2(stat->static_type_name() << "  "<< stat->toString());
    auto mi = P4::MethodInstance::resolve(stat->methodCall, refMap, typeMap);
    auto em = mi->to<P4::ExternMethod>();
    if(em != nullptr){
        if (em->originalExternType->name.name == P4::P4CoreLibrary::instance.packetIn.name &&
            em->method->name.name == P4::P4CoreLibrary::instance.packetIn.extract.name) {
            auto mc = stat->methodCall;
            auto arg = mc->arguments->at(0);
            LOG1(stat);
        }
    }
    return true;
}
bool DoStaticEvaluation::preorder(const IR::MethodCallExpression *prog){
    LOG3(prog->static_type_name() << "  "<< prog->toString());
    return true;
}

} // namespace FPGA