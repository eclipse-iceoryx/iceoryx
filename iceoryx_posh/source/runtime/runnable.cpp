// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/runtime/runnable.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/runnable_data.hpp"
#include "iceoryx_posh/internal/runtime/runnable_property.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace runtime
{
Runnable::Runnable(const iox::cxx::CString100& runnableName) noexcept
    : m_data(PoshRuntime::getInstance().createRunnable(RunnableProperty(runnableName, 0)))
{
}

Runnable::~Runnable() noexcept
{
}

Runnable::Runnable(Runnable&& rhs) noexcept
{
    *this = std::move(rhs);
}

Runnable& Runnable::operator=(Runnable&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_data = rhs.m_data;
        rhs.m_data = nullptr;
    }

    return *this;
}

cxx::CString100 Runnable::getRunnableName() const noexcept
{
    return m_data->m_runnable;
}

cxx::CString100 Runnable::getProcessName() const noexcept
{
    return m_data->m_process;
}

} // namespace runtime
} // namespace iox
