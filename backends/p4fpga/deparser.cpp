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
#include "backends/p4fpga/deparser.h"
#include "ir/ir-generated.h"
#include "lib/json.h"
#include "p4fpga/JsonObjects.h"
#include <string>

namespace FPGA {
Util::JsonObject* DeparserConverter::convertDeparser(const IR::P4Control* ctrl){
    Util::JsonObject* dep = new Util::JsonObject();
    dep->emplace("name", ctrl->getName());
    Util::JsonArray* body = convertDeparserBody(&ctrl->body->components);
    dep->emplace("machine", body);
    return dep;
}

Util::JsonArray* DeparserConverter::convertDeparserBody(const IR::Vector<IR::StatOrDecl>* body){
    Util::JsonArray* jsonBody = new Util::JsonArray();
    for (auto s : *body) {
        if (auto block = s->to<IR::BlockStatement>()) {
            Util::JsonObject* tmp = new Util::JsonObject();
            auto nom = "b"+std::to_string(uniqueID);
            uniqueID++;
            tmp->emplace(nom, convertDeparserBody(&block->components));
            jsonBody->append(tmp);
            uniqueID++;
            continue;
        }else{
            if(s->is<IR::MethodCallStatement>()){
                auto mc = s->to<IR::MethodCallStatement>()->methodCall;
                auto arg = mc->arguments->at(0);
                jsonBody->append(arg->toString());
            }
            else jsonBody->append(s->toString());
        }
    }
    return jsonBody;
}
bool DeparserConverter::preorder(const IR::P4Control* control) {
    auto deparserJson = convertDeparser(control);
    json->setDeparser(deparserJson);
    return false;
}
}