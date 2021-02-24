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

#include "backends/p4fpga/p4fpga.h"
#include "common/options.h"
#include "ir/ir-generated.h"
#include "backends/p4fpga/deparser.h"
#include <iostream>

namespace FPGA{
    FPGABackend::FPGABackend(FPGA::P4FpgaOptions& options) : 
                    options(options) {
        json = new FPGA::FPGAJson(options);
        busOutWidth = options.outBusWidth;
    }

    void FPGABackend::convert(const IR::ToplevelBlock* tlb){
        auto main = tlb->getMain();
        auto deparser = main->findParameterValue("dep");
        if (!main) return;
       // for(auto s : deparser->to<IR::P4Control>()->body->components)
       auto depConv = new DeparserConverter(json);
       deparser->to<IR::ControlBlock>()->container->apply(*depConv);
       std::cout << deparser << std::endl; // DEVHELP 
    }
    
}
