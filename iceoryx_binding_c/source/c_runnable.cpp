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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/runnable_data.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/runnable.hpp"

using namespace iox;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/runnable.h"
}

class RunnableBindingExtension : public iox::runtime::Runnable
{
  public:
    RunnableBindingExtension(RunnableData* const data)
        : Runnable(data)
    {
    }

    ~RunnableBindingExtension()
    {
        m_data = nullptr;
    }

    void destroy()
    {
        m_data->m_toBeDestroyed.store(true, std::memory_order_relaxed);
    }
};

iox_runnable_t iox_runnable_create(const char* const runnableName)
{
    return PoshRuntime::getInstance().createRunnable(
        RunnableProperty(RunnableName_t(iox::cxx::TruncateToCapacity, runnableName), 0u));
}

void iox_runnable_destroy(iox_runnable_t const self)
{
    RunnableBindingExtension(self).destroy();
}

uint64_t iox_runnable_get_name(iox_runnable_t const self, char* const name, const uint64_t nameCapacity)
{
    auto nameAsString = RunnableBindingExtension(self).getRunnableName();
    strncpy(name, nameAsString.c_str(), nameCapacity);
    return nameAsString.size();
}

uint64_t iox_runnable_get_process_name(iox_runnable_t const self, char* const name, const uint64_t nameCapacity)
{
    auto nameAsString = RunnableBindingExtension(self).getProcessName();
    strncpy(name, nameAsString.c_str(), nameCapacity);
    return nameAsString.size();
}

