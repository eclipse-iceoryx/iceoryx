// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iox/detail/convert.hpp"
#include "iox/detail/system_configuration.hpp"
#include "iox/logging.hpp"

#include "iceoryx_platform/signal.hpp"
#include "iceoryx_platform/unistd.hpp"

namespace iox
{
namespace roudi
{
constexpr access_rights PosixShmMemoryProvider::SHM_MEMORY_PERMISSIONS;

PosixShmMemoryProvider::PosixShmMemoryProvider(const ShmName_t& shmName,
                                               const DomainId domainId,
                                               const AccessMode accessMode,
                                               const OpenMode openMode) noexcept
    : m_shmName(shmName)
    , m_domainId(domainId)
    , m_accessMode(accessMode)
    , m_openMode(openMode)
{
}

PosixShmMemoryProvider::~PosixShmMemoryProvider() noexcept
{
    if (isAvailable())
    {
        destroy().or_else([](auto) { IOX_LOG(WARN, "failed to cleanup POSIX shared memory provider resources"); });
    }
}

expected<void*, MemoryProviderError> PosixShmMemoryProvider::createMemory(const uint64_t size,
                                                                          const uint64_t alignment) noexcept
{
    if (alignment > detail::pageSize())
    {
        return err(MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE);
    }

    if (!PosixSharedMemoryObjectBuilder()
             .name(concatenate(iceoryxResourcePrefix(m_domainId, ResourceType::ICEORYX_DEFINED), m_shmName))
             .memorySizeInBytes(size)
             .accessMode(m_accessMode)
             .openMode(m_openMode)
             .permissions(SHM_MEMORY_PERMISSIONS)
             .create()
             .and_then([this](auto& sharedMemoryObject) { m_shmObject.emplace(std::move(sharedMemoryObject)); }))
    {
        return err(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    auto baseAddress = m_shmObject->getBaseAddress();
    if (baseAddress == nullptr)
    {
        return err(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    return ok(baseAddress);
}

expected<void, MemoryProviderError> PosixShmMemoryProvider::destroyMemory() noexcept
{
    m_shmObject.reset();
    return ok();
}

} // namespace roudi
} // namespace iox
