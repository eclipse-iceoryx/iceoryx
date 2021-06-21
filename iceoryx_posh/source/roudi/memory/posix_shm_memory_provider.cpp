// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/system_configuration.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/platform/signal.hpp"
#include "iceoryx_hoofs/platform/unistd.hpp"

namespace iox
{
namespace roudi
{
PosixShmMemoryProvider::PosixShmMemoryProvider(const ShmName_t& shmName,
                                               const posix::AccessMode accessMode,
                                               const posix::OpenMode openMode) noexcept
    : m_shmName(shmName)
    , m_accessMode(accessMode)
    , m_openMode(openMode)
{
}

PosixShmMemoryProvider::~PosixShmMemoryProvider() noexcept
{
    if (isAvailable())
    {
        destroy().or_else([](auto) { LogWarn() << "failed to cleanup POSIX shared memory provider resources"; });
    }
}

cxx::expected<void*, MemoryProviderError> PosixShmMemoryProvider::createMemory(const uint64_t size,
                                                                               const uint64_t alignment) noexcept
{
    if (alignment > posix::pageSize())
    {
        return cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE);
    }

    posix::SharedMemoryObject::create(m_shmName, size, m_accessMode, m_openMode, nullptr)
        .and_then([this](auto& sharedMemoryObject) {
            sharedMemoryObject.finalizeAllocation();
            m_shmObject.emplace(std::move(sharedMemoryObject));
        });

    if (!m_shmObject.has_value())
    {
        return cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    auto baseAddress = m_shmObject->getBaseAddress();
    if (baseAddress == nullptr)
    {
        return cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    return cxx::success<void*>(baseAddress);
}

cxx::expected<MemoryProviderError> PosixShmMemoryProvider::destroyMemory() noexcept
{
    m_shmObject.reset();
    return cxx::success<void>();
}

} // namespace roudi
} // namespace iox
