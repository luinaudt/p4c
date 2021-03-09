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
#ifndef BACKENDS_FPGA_DEPARSERGRAPHCLOSER_H_
#define BACKENDS_FPGA_DEPARSERGRAPHCLOSER_H_

#include "ir/ir.h"
#include "ir/visitor.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "p4/typeMap.h"
#include <iostream>

namespace FPGA {
    class DeparserGraphCloser : public Transform{
        P4::P4CoreLibrary&     corelib;
        P4::ReferenceMap* refMap;
        P4::TypeMap* typeMap;
        
        protected:
            const IR::Node* convertBody(const IR::Vector<IR::StatOrDecl>* body);
            
        public:
        const IR::Node* preorder(IR::P4Control* ctrl);
        
        const IR::Node* postorder(IR::MethodCallStatement* s);
        const IR::Node* postorder(IR::P4Control* ctrl);
        const IR::Node* preorder(IR::IfStatement* cond);
        const IR::Node* preorder(IR::StatOrDecl* s);
        explicit DeparserGraphCloser(P4::ReferenceMap* refMap, P4::TypeMap* typeMap) 
        :  corelib(P4::P4CoreLibrary::instance), refMap(refMap), typeMap(typeMap)
            { CHECK_NULL(refMap); CHECK_NULL(typeMap); setName("CloseDeparserGraph");}
        };
}

#endif /* BACKENDS_FPGA_DEPARSERGRAPHCLOSER_H_ */