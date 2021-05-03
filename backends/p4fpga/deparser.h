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
#ifndef BACKENDS_P4FPGA_DEPARSER_H_
#define BACKENDS_P4FPGA_DEPARSER_H_

#include <cstdint>
#include <vector>
#include "ir/ir.h"
#include "ir/vector.h"
#include "ir/visitor.h"
#include "lib/cstring.h"
#include "lib/json.h"
#include "frontends/p4/typeMap.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "frontends/p4/coreLibrary.h"
#include "backends/p4fpga/JsonObjects.h"
#include "lib/ordered_set.h"


namespace FPGA {

class DeparserConverter : public Inspector {
    typedef struct state{
        cstring     name;       // hdrName
        uint64_t    nbBits;      // nbBit to emit
        uint64_t    startPos;   // position first bit
    } state_t;

    cstring                name;
    uint64_t               nbEmitBits;
    uint64_t               outputBusWidth;
    P4::P4CoreLibrary&     corelib;
    FPGA::FPGAJson*        json;
    std::vector<cstring>*  condList;  // list of condition
    ordered_set<cstring>*  state_set;  // list of states
    ordered_set<cstring>*  currentState;
    ordered_set<cstring>*  previousState;
    ordered_set<cstring>*  links_set;
    Util::JsonArray*       links;
    P4::ReferenceMap*      refMap;
    P4::TypeMap*           typeMap;

 protected:
        Util::JsonObject* convertDeparser(const IR::P4Control* ctrl);
        void insertState(state_t info);
        void insertTransition();  // links each previous state with each current states

 public:
        bool preorder(const IR::P4Control* ctrl) override;
        bool preorder(const IR::IfStatement* cond) override;
        bool preorder(const IR::MethodCallStatement* s) override;
        void postorder(const IR::P4Control* ctrl) override;

        explicit DeparserConverter(FPGA::FPGAJson* json, P4::ReferenceMap* refMap,
                                P4::TypeMap* typeMap, uint64_t outBusWidth,
                                cstring name = "deparser")
            : name(name), corelib(P4::P4CoreLibrary::instance), json(json),
            refMap(refMap), typeMap(typeMap), outputBusWidth(outBusWidth){
            visitDagOnce = false;
            setName("DeparserConverter");
        }
};
}  // namespace FPGA
#endif  // BACKENDS_P4FPGA_DEPARSER_H_
