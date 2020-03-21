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

#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/internal/posix_wrapper/system_configuration.hpp"

#include "iceoryx_utils/platform/signal.hpp"
#include "iceoryx_utils/platform/unistd.hpp"

namespace iox
{
namespace roudi
{
namespace
{
void sigbusHandler(int) noexcept
{
    char msg[] =
        "\033[0;1;97;41mFatal error:\033[m the available memory is insufficient. Cannot allocate mempools in shared "
        "memory. Please make sure that enough memory is available. For this, consider also the memory which is "
        "required for the [/iceoryx_mgmt] segment. Please refer to share/doc/iceoryx/FAQ.md in your release delivery.";
    size_t len = strlen(msg);
    write(STDERR_FILENO, msg, len);
    _exit(EXIT_FAILURE);
}
} // namespace

PosixShmMemoryProvider::PosixShmMemoryProvider(const ShmNameString& shmName,
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

    /// @todo the SIGBUS handler could maybe be moved to the RouDiMemoryManager

    // register signal handler for SIGBUS
    struct sigaction oldAct;
    struct sigaction newAct;
    sigemptyset(&newAct.sa_mask);
    newAct.sa_handler = sigbusHandler;
    newAct.sa_flags = 0;
    if (cxx::makeSmartC(sigaction, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGBUS, &newAct, &oldAct)
            .hasErrors())
    {
        LogFatal() << "Could not set signal handler for SIGBUS!";
        std::terminate();
    }

    // create and map a shared memory region
    m_shmObject = posix::SharedMemoryObject::create(m_shmName.c_str(), size, m_accessMode, m_ownership, nullptr);

    // unregister signal handler
    if (cxx::makeSmartC(sigaction, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGBUS, &oldAct, nullptr)
            .hasErrors())
    {
        LogFatal() << "Could not reset signal handler for SIGBUS!";
        std::terminate();
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
