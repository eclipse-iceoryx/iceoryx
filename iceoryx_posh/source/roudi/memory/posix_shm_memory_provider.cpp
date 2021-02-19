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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/internal/posix_wrapper/system_configuration.hpp"

#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/platform/signal.hpp"
#include "iceoryx_utils/platform/unistd.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

namespace iox
{
namespace roudi
{
namespace
{
void sigbusHandler(int32_t) noexcept
{
    char msg[] =
        "\033[0;1;97;41mFatal error:\033[m the available memory is insufficient. Cannot allocate mempools in shared "
        "memory. Please make sure that enough memory is available. For this, consider also the memory which is "
        "required for the [/iceoryx_mgmt] segment. Please refer to share/doc/iceoryx/FAQ.md in your release delivery.";
    size_t len = strlen(msg);
    DISCARD_RESULT(write(STDERR_FILENO, msg, len));
    _exit(EXIT_FAILURE);
}
} // namespace

PosixShmMemoryProvider::PosixShmMemoryProvider(const ShmName_t& shmName,
                                               const posix::AccessMode accessMode,
                                               const posix::OwnerShip ownership) noexcept
    : m_shmName(shmName)
    , m_accessMode(accessMode)
    , m_ownership(ownership)
{
}

PosixShmMemoryProvider::~PosixShmMemoryProvider() noexcept
{
    destroy();
}

cxx::expected<void*, MemoryProviderError> PosixShmMemoryProvider::createMemory(const uint64_t size,
                                                                               const uint64_t alignment) noexcept
{
    auto pageSize = posix::pageSize();
    if (!pageSize.has_value())
    {
        return cxx::error<MemoryProviderError>(MemoryProviderError::PAGE_SIZE_CHECK_ERROR);
    }

    if (alignment > posix::pageSize().value())
    {
        return cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE);
    }

    // register temporary signal handler for SIGBUS
    {
        auto signalGuard = posix::registerSignalHandler(posix::Signal::BUS, sigbusHandler);

        // create and map a shared memory region
        m_shmObject = posix::SharedMemoryObject::create(m_shmName.c_str(), size, m_accessMode, m_ownership, nullptr);
    }

    if (!m_shmObject.has_value())
    {
        return cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    auto baseAddress = m_shmObject->getBaseAddress();
    if (baseAddress == nullptr)
    {
        return cxx::error<MemoryProviderError>(MemoryProviderError::MEMORY_CREATION_FAILED);
    }

    m_shmObject->finalizeAllocation();

    return cxx::success<void*>(baseAddress);
}

cxx::expected<MemoryProviderError> PosixShmMemoryProvider::destroyMemory() noexcept
{
    m_shmObject.reset();
    return cxx::success<void>();
}

} // namespace roudi
} // namespace iox
