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

#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_posh/roudi/memory/memory_provider.hpp"

namespace iox
{
namespace roudi
{
RouDiMemoryManager::~RouDiMemoryManager() noexcept
{
    destroyMemory();
}

cxx::expected<RouDiMemoryManagerError> RouDiMemoryManager::addMemoryProvider(MemoryProvider* memoryProvider) noexcept
{
    if (m_memoryProvider.push_back(memoryProvider))
    {
        return cxx::success<>();
    }
    return cxx::error<RouDiMemoryManagerError>(RouDiMemoryManagerError::MEMORY_PROVIDER_EXHAUSTED);
}

cxx::expected<RouDiMemoryManagerError> RouDiMemoryManager::createAndAnnounceMemory() noexcept
{
    if (m_memoryProvider.empty())
    {
        return cxx::error<RouDiMemoryManagerError>(RouDiMemoryManagerError::NO_MEMORY_PROVIDER_PRESENT);
    }

    for (auto memoryProvider : m_memoryProvider)
    {
        auto result = memoryProvider->create();
        if (result.has_error())
        {
            LogError() << "Could not create memory: MemoryProviderError = "
                       << MemoryProvider::getErrorString(result.get_error());
            return cxx::error<RouDiMemoryManagerError>(RouDiMemoryManagerError::MEMORY_CREATION_FAILED);
        }
    }

    for (auto memoryProvider : m_memoryProvider)
    {
        memoryProvider->announceMemoryAvailable();
    }

    return cxx::success<>();
}

cxx::expected<RouDiMemoryManagerError> RouDiMemoryManager::destroyMemory() noexcept
{
    cxx::expected<RouDiMemoryManagerError> result = cxx::success<void>();
    for (auto memoryProvider : m_memoryProvider)
    {
        auto destructionResult = memoryProvider->destroy();
        if (destructionResult.has_error() && destructionResult.get_error() != MemoryProviderError::MEMORY_NOT_AVAILABLE)
        {
            LogError() << "Could not destroy memory provider! Error: "
                       << static_cast<uint64_t>(destructionResult.get_error());
            /// @note do not return on first error but try to cleanup the remaining resources
            if (!result.has_error())
            {
                result = cxx::error<RouDiMemoryManagerError>(RouDiMemoryManagerError::MEMORY_DESTRUCTION_FAILED);
            }
        }
    }

    return result;
}

} // namespace roudi
} // namespace iox
