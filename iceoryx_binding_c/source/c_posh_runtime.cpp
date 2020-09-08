// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "iceoryx_posh/runtime/posh_runtime.hpp"

using namespace iox;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/posh_runtime.h"
}

void iox_runtime_register(const char* const name)
{
    PoshRuntime::getInstance(name);
}

uint64_t iox_runtime_get_instance_name(char* const name, const uint64_t nameLength)
{
    auto instanceName = PoshRuntime::getInstance().getInstanceName();
    uint64_t instanceNameSize = instanceName.size();
    strncpy(name, instanceName.c_str(), std::min(nameLength, instanceNameSize));
    return instanceNameSize;
}

