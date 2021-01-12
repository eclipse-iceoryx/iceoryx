// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#include "iceoryx_binding_c/runtime.h"
}

void iox_runtime_init(const char* const name)
{
    if (name == nullptr)
    {
        LogError() << "Application name is a nullptr!";
        std::terminate();
    }
    else if (strnlen(name, iox::MAX_PROCESS_NAME_LENGTH + 1) > MAX_PROCESS_NAME_LENGTH)
    {
        LogError() << "Application name has more than 100 characters!";
        std::terminate();
    }

    PoshRuntime::initRuntime(ProcessName_t(iox::cxx::TruncateToCapacity, name));
}

uint64_t iox_runtime_get_instance_name(char* const name, const uint64_t nameLength)
{
    if (name == nullptr)
    {
        return 0U;
    }

    auto instanceName = PoshRuntime::getInstance().getInstanceName();
    std::strncpy(name, instanceName.c_str(), nameLength);
    name[nameLength - 1U] = '\0'; // strncpy doesn't add a null-termination if destination is smaller than source

    return instanceName.size();
}
