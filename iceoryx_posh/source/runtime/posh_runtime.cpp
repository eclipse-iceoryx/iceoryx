// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"

#include <cstdint>

namespace iox
{
namespace runtime
{
PoshRuntime::factory_t& PoshRuntime::getRuntimeFactory() noexcept
{
    static factory_t runtimeFactory = PoshRuntimeImpl::defaultRuntimeFactory;
    return runtimeFactory;
}

void PoshRuntime::setRuntimeFactory(const factory_t& factory) noexcept
{
    if (factory)
    {
        PoshRuntime::getRuntimeFactory() = factory;
    }
    else
    {
        LogFatal() << "Cannot set runtime factory. Passed factory must not be empty!";
        errorHandler(Error::kPOSH__RUNTIME_FACTORY_IS_NOT_SET);
    }
}

PoshRuntime& PoshRuntime::defaultRuntimeFactory(cxx::optional<const RuntimeName_t*> name) noexcept
{
    static PoshRuntimeImpl instance(name);
    return instance;
}

// singleton access
PoshRuntime& PoshRuntime::getInstance() noexcept
{
    return getInstance(cxx::nullopt);
}

PoshRuntime& PoshRuntime::initRuntime(const RuntimeName_t& name) noexcept
{
    return getInstance(cxx::make_optional<const RuntimeName_t*>(&name));
}

PoshRuntime& PoshRuntime::getInstance(cxx::optional<const RuntimeName_t*> name) noexcept
{
    return getRuntimeFactory()(name);
}

PoshRuntime::PoshRuntime(cxx::optional<const RuntimeName_t*> name) noexcept
    : m_appName(verifyInstanceName(name))
{
    if (cxx::isCompiledOn32BitSystem())
    {
        LogWarn() << "Running applications on 32-bit architectures is not supported! Use at your own risk!";
    }

    /// @todo here we could get the LogLevel and LogMode and set it on the LogManager
}

const RuntimeName_t& PoshRuntime::verifyInstanceName(cxx::optional<const RuntimeName_t*> name) noexcept
{
    if (!name.has_value())
    {
        LogError() << "Cannot initialize runtime. Application name has not been specified!";
        errorHandler(Error::kPOSH__RUNTIME_NO_NAME_PROVIDED, nullptr, ErrorLevel::FATAL);
    }
    else if (name.value()->empty())
    {
        LogError() << "Cannot initialize runtime. Application name must not be empty!";
        errorHandler(Error::kPOSH__RUNTIME_NAME_EMPTY, nullptr, ErrorLevel::FATAL);
    }
    else if (name.value()->c_str()[0] == '/')
    {
        LogError() << "Cannot initialize runtime. Please remove leading slash from Application name " << *name.value();
        errorHandler(Error::kPOSH__RUNTIME_LEADING_SLASH_PROVIDED, nullptr, ErrorLevel::FATAL);
    }

    return *name.value();
}

RuntimeName_t PoshRuntime::getInstanceName() const noexcept
{
    return m_appName;
}

void PoshRuntime::shutdown() noexcept
{
    m_shutdownRequested.store(true, std::memory_order_relaxed);
}

} // namespace runtime
} // namespace iox
