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

namespace FPGA {
    bool DoStaticEvaluation::preorder(const IR::ToplevelBlock *tlb){
        LOG1("visiting program according to execution order");
        auto main = tlb->getMain();
        auto param = main->getConstructorParameters();
        for(auto i: *param){
            auto arg = main->getParameterValue(i->getName());
            if(arg->is<IR::ParserBlock>()){
                auto block = arg->to<IR::ParserBlock>()->container;
                visit(block);
            }
            else if(arg->is<IR::ControlBlock>()){
                auto block = arg->to<IR::ControlBlock>()->container;
                visit(block);
            }
            else {
                LOG1("autre " << arg->toString());
            }
        }
        return true;
    }

    bool DoStaticEvaluation::preorder(const IR::ParameterList *params){
        for(auto i : *params){
            LOG1("Param " << i->getName() << " " << typeMap->getType(i)->node_type_name());
            LOG1("Direction " << i->direction);
        }
        return true;
    }
    bool DoStaticEvaluation::preorder(const IR::P4Parser *block){
        LOG1("visiting " << block->static_type_name() << " " << block->getName() << IndentCtl::indent);
        return true;
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

    bool DoStaticEvaluation::preorder(const IR::MethodCallStatement *prog){
        LOG2(prog->static_type_name() << "  "<< prog->toString());
        return true;
    }
    bool DoStaticEvaluation::preorder(const IR::MethodCallExpression *prog){
        LOG2(prog->static_type_name() << "  "<< prog->toString());
        return true;
    }
    
    
}