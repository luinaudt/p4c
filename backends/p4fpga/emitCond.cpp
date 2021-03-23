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
#include "lib/log.h"

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
            auto hdrT = typeMap->getType(arg0, true);
            if(hdrT->is<IR::Type_Header>()){
                auto st = arg0->expression;
                auto mc = new IR::Member(s->srcInfo, 
                                        st, IR::Type_Header::isValid);
                auto cond = new IR::MethodCallExpression(mc, new IR::Vector<IR::Argument>());
                auto newCond = new IR::IfStatement(cond, s, nullptr);
                LOG1("Converting " << s );
                LOG2("to\n" << newCond);
                return newCond;
            }
        }
        /*
        // FIXME - names and doc
        // packet_in externs
        if(em->originalExternType->name.name == P4::P4CoreLibrary::instance.packetIn.name){
            // convert extrat to extract, hdr.setValid()
            if(em->method->name.name == P4::P4CoreLibrary::instance.packetIn.extract.name) {
                auto arg0 = em->expr->arguments->at(0);
                auto hdrT = typeMap->getType(arg0, true);
                if(hdrT->is<IR::Type_Header>()){
                    auto st = arg0->expression;
                    auto mc = new IR::Member(s->srcInfo, st, IR::Type_Header::setValid);
                    auto call = new IR::MethodCallExpression(mc, new IR::Vector<IR::Argument>());
                    auto newVec = new IR::IndexedVector<IR::StatOrDecl>();
                    newVec->push_back(s);
                    newVec->push_back(new IR::MethodCallStatement(call));
                    LOG1("Converting " << s );
                    LOG2("to\n" << newVec);
                    return new IR::BlockStatement(*newVec);
                }
            }
        }*/
    }
        return s;
    }
}