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
#include "backends/p4fpga/emitCond.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/p4/methodInstance.h"
#include "ir/ir-generated.h"
#include "ir/ir.h"

namespace FPGA{

    const IR::Node* EmitCond::postorder(IR::MethodCallStatement* s){
        auto mi = P4::MethodInstance::resolve(s->methodCall, refMap, typeMap);
         if (auto em = mi->to<P4::ExternMethod>()) {
          if (em->originalExternType->name.name == P4::P4CoreLibrary::instance.packetOut.name &&
            em->method->name.name == P4::P4CoreLibrary::instance.packetOut.emit.name) {
            if (em->expr->arguments->size() != 1) {
                ::error(ErrorType::ERR_UNEXPECTED, "%1%: expected exactly 1 argument", s);
                return s;
            }
            auto arg0 = em->expr->arguments->at(0);
            std::cout << "emitCond0" << std::endl;
            auto hdrT = typeMap->getType(arg0, true);
            if(hdrT->is<IR::Type_Header>()){
                std::cout << "emitCond Hdr" << std::endl;
                auto st = arg0->expression;
                auto mc = new IR::Member(s->srcInfo, 
                                        st, IR::Type_Header::isValid);
                std::cout << "emitCond member gen" << std::endl;
                auto cond = new IR::MethodCallExpression(mc, new IR::Vector<IR::Argument>());
                auto newCond = new IR::IfStatement(cond, s, nullptr);
                return newCond;
            }
        }
        }
        return s;   
    }
}