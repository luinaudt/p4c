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

#include "backends/p4fpga/JsonObjects.h"
#include "lib/json.h"
namespace FPGA {

    FPGAJson::FPGAJson(FPGA::P4FpgaOptions& options){
        toplevel = new Util::JsonObject();
        parser   = new Util::JsonObject();
        control  = new Util::JsonObject();
        // place top level parameters.
        toplevel->emplace("outputBus", options.outBusWidth);
        toplevel->emplace("parser", parser);
        toplevel->emplace("control", control);
    }

    void FPGAJson::setDeparser(Util::JsonObject* jo){
        deparser = jo;
        toplevel->emplace("deparser", deparser);
    }
}
