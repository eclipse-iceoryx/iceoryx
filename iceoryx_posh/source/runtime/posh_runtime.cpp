// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iox/atomic.hpp"
#include "iox/detail/system_configuration.hpp"
#include "iox/filesystem.hpp"
#include "iox/logging.hpp"

#include <cstdint>
#include <new>
#include <type_traits>

namespace iox
{
namespace runtime
{
namespace
{

// A refcount for use in getLifetimeParticipant(). The refcount being > 0 does not
// necessarily mean that the runtime is initialized yet, it only controls the point
// at which the runtime is destroyed.
concurrent::Atomic<uint64_t>& poshRuntimeStaticRefCount()
{
    static concurrent::Atomic<uint64_t> s_refcount{0U};
    return s_refcount;
}
// Tracks whether the refcount lifetime mechanism is used by the factory function.
// Only the PoshRuntimeImpl factory uses this mechanism, other factories use
// regular static variables.
// Tracking this is necessary to avoid calling the destructor twice for the other
// classes that are not PoshRuntimeImpl, and also guards against the destructor
// being called on a non-existent object in the case where a lifetime participant
// goes out of scope before the PoshRuntimeImpl instance was constructed.
concurrent::Atomic<bool>& poshRuntimeNeedsManualDestruction()
{
    static concurrent::Atomic<bool> s_needsManualDestruction{false};
    return s_needsManualDestruction;
}

} // anonymous namespace

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
        IOX_LOG(FATAL, "Cannot set runtime factory. Passed factory must not be empty!");
        IOX_REPORT_FATAL(PoshError::POSH__RUNTIME_FACTORY_IS_NOT_SET);
    }
}

PoshRuntime& PoshRuntime::defaultRuntimeFactory(optional<const RuntimeName_t*> name) noexcept
{
    // Manual construction and destruction of the PoshRuntimeImpl, inspired by
    // the nifty counter idiom.
    static typename std::aligned_storage<sizeof(PoshRuntimeImpl), alignof(PoshRuntimeImpl)>::type buf;
    // This is the primary lifetime participant. It ensures that, even if getLifetimeParticipant()
    // is never called, the runtime has the same lifetime as a regular static variable.
    static ScopeGuard staticLifetimeParticipant = [](auto name) {
        new (&buf) PoshRuntimeImpl(name);
        poshRuntimeNeedsManualDestruction() = true;
        return getLifetimeParticipant();
    }(name);
    return reinterpret_cast<PoshRuntimeImpl&>(buf);
}

// singleton access
PoshRuntime& PoshRuntime::getInstance() noexcept
{
    return getInstance(nullopt);
}

PoshRuntime& PoshRuntime::initRuntime(const RuntimeName_t& name) noexcept
{
    return getInstance(make_optional<const RuntimeName_t*>(&name));
}

PoshRuntime& PoshRuntime::getInstance(optional<const RuntimeName_t*> name) noexcept
{
    return getRuntimeFactory()(name);
}

ScopeGuard PoshRuntime::getLifetimeParticipant() noexcept
{
    return ScopeGuard([]() { ++poshRuntimeStaticRefCount(); },
                      []() {
                          if (0 == --poshRuntimeStaticRefCount() && poshRuntimeNeedsManualDestruction())
                          {
                              getInstance().~PoshRuntime();
                          }
                      });
}

PoshRuntime::PoshRuntime(optional<const RuntimeName_t*> name) noexcept
    : m_appName(verifyInstanceName(name))
{
    if (detail::isCompiledOn32BitSystem())
    {
        IOX_LOG(WARN, "Running applications on 32-bit architectures is experimental! Use at your own risk!");
    }
}

const RuntimeName_t& PoshRuntime::verifyInstanceName(optional<const RuntimeName_t*> name) noexcept
{
    if (!name.has_value())
    {
        IOX_LOG(FATAL, "Cannot initialize runtime. Application name has not been specified!");
        IOX_REPORT_FATAL(PoshError::POSH__RUNTIME_NO_NAME_PROVIDED);
    }
    else if (!isValidFileName(**name))
    {
        IOX_LOG(FATAL,
                "Cannot initialize runtime. The application name \""
                    << **name << "\" is not a valid platform-independent file name.");
        IOX_REPORT_FATAL(PoshError::POSH__RUNTIME_NAME_NOT_VALID_FILE_NAME);
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
