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
#ifndef BACKENDS_FPGA_PACKETEXTERNTRANSLATE_H_
#define BACKENDS_FPGA_PACKETEXTERNTRANSLATE_H_

#include "ir/ir.h"
#include "ir/visitor.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "midend/expandEmit.h"
#include "p4/typeChecking/typeChecker.h"
#include "p4/typeMap.h"

#include <iostream>
namespace FPGA{
    class EmitCond : public Transform{
        /** Convert emit to condition :
            emit(hdr) -> if (hdr.isValid()) emit(hdr);
        */
        P4::ReferenceMap* refMap;
        P4::TypeMap* typeMap;
    public:
        EmitCond(P4::ReferenceMap* refMap, P4::TypeMap* typeMap):
                refMap(refMap), typeMap(typeMap)
        { CHECK_NULL(refMap); CHECK_NULL(typeMap); setName("EmitCond"); }
        const IR::Node* postorder(IR::MethodCallStatement* statement) override;
    };

    /** Convert packetin.extract(hdr) extern to 
    packetin.extract(hdr)
    hdr.setValid()
    TODO - add support for other externs if necessary
    This should help for header analysis and validity
    */
    class PacketInTranslate : public Transform{
        P4::ReferenceMap* refMap;
        P4::TypeMap* typeMap;
        public:
        PacketInTranslate(P4::ReferenceMap* refMap, P4::TypeMap* typeMap):
                        refMap(refMap), typeMap(typeMap)
        { CHECK_NULL(refMap); CHECK_NULL(typeMap); setName("PacketInTranslate"); }
        const IR::Node* postorder(IR::MethodCallStatement* statement) override;

    };
    /**Pass manager to convert standard externs of packetIn and PacketOut

    */
    class PacketExternTranslate : public PassManager{
    public:
    explicit PacketExternTranslate(P4::ReferenceMap* refMap, P4::TypeMap* typeMap, 
                                   bool transformPacketIn, bool transformPacketOut)
            {
                if(transformPacketOut){
                    passes.push_back(new P4::TypeChecking (refMap, typeMap));
                    passes.push_back(new P4::ExpandEmit (refMap, typeMap));
                    passes.push_back(new P4::TypeChecking (refMap, typeMap));
                    passes.push_back(new EmitCond(refMap, typeMap));
                }
                if(transformPacketIn){
                    passes.push_back(new P4::TypeChecking (refMap, typeMap));
                    passes.push_back(new PacketInTranslate(refMap, typeMap));
                }
                setName("PacketExternTranslate"); 
            }
    };

}

#endif /* BACKENDS_FPGA_PACKETEXTERNTRANSLATE_H_ */