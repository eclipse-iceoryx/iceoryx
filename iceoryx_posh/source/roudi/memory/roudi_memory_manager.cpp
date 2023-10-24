// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"

#include "iceoryx_posh/roudi/memory/memory_provider.hpp"
#include "iox/logging.hpp"

namespace iox
{
namespace roudi
{
iox::log::LogStream& operator<<(iox::log::LogStream& logstream, const RouDiMemoryManagerError& error) noexcept
{
    switch (error)
    {
    case RouDiMemoryManagerError::MEMORY_PROVIDER_EXHAUSTED:
        logstream << "MEMORY_PROVIDER_EXHAUSTED";
        break;
    case RouDiMemoryManagerError::NO_MEMORY_PROVIDER_PRESENT:
        logstream << "NO_MEMORY_PROVIDER_PRESENT";
        break;
    case RouDiMemoryManagerError::MEMORY_CREATION_FAILED:
        logstream << "MEMORY_CREATION_FAILED";
        break;
    case RouDiMemoryManagerError::MEMORY_DESTRUCTION_FAILED:
        logstream << "MEMORY_DESTRUCTION_FAILED";
        break;
    default:
        logstream << "ROUDI_MEMEMORY_ERROR_UNDEFINED";
        break;
    }
    return logstream;
}

RouDiMemoryManager::~RouDiMemoryManager() noexcept
{
    destroyMemory().or_else([](auto) { IOX_LOG(WARN, "Failed to cleanup RouDiMemoryManager in destructor."); });
}

expected<void, RouDiMemoryManagerError> RouDiMemoryManager::addMemoryProvider(MemoryProvider* memoryProvider) noexcept
{
    if (m_memoryProvider.push_back(memoryProvider))
    {
        return ok();
    }
    return err(RouDiMemoryManagerError::MEMORY_PROVIDER_EXHAUSTED);
}

expected<void, RouDiMemoryManagerError> RouDiMemoryManager::createAndAnnounceMemory() noexcept
{
    if (m_memoryProvider.empty())
    {
        return err(RouDiMemoryManagerError::NO_MEMORY_PROVIDER_PRESENT);
    }

    for (auto memoryProvider : m_memoryProvider)
    {
        auto result = memoryProvider->create();
        if (result.has_error())
        {
            IOX_LOG(
                ERROR,
                "Could not create memory: MemoryProviderError = " << MemoryProvider::getErrorString(result.error()));
            return err(RouDiMemoryManagerError::MEMORY_CREATION_FAILED);
        }
    }

    for (auto memoryProvider : m_memoryProvider)
    {
        memoryProvider->announceMemoryAvailable();
    }

    return ok();
}

expected<void, RouDiMemoryManagerError> RouDiMemoryManager::destroyMemory() noexcept
{
    expected<void, RouDiMemoryManagerError> result = ok();
    for (auto memoryProvider : m_memoryProvider)
    {
        auto destructionResult = memoryProvider->destroy();
        if (destructionResult.has_error() && destructionResult.error() != MemoryProviderError::MEMORY_NOT_AVAILABLE)
        {
            IOX_LOG(ERROR,
                    "Could not destroy memory provider! Error: " << static_cast<uint64_t>(destructionResult.error()));
            /// @note do not return on first error but try to cleanup the remaining resources
            if (!result.has_error())
            {
                result = err(RouDiMemoryManagerError::MEMORY_DESTRUCTION_FAILED);
            }
        }
    }

    return result;
}

} // namespace roudi
} // namespace iox
