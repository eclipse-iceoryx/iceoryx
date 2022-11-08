// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_hoofs/cxx/attributes.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_handler.hpp"
#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/unistd.hpp"

#include <bitset>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>

namespace iox
{
namespace posix
{
constexpr const void* const SharedMemoryObject::NO_ADDRESS_HINT;
constexpr uint64_t SIGBUS_ERROR_MESSAGE_LENGTH = 1024U + platform::IOX_MAX_SHM_NAME_LENGTH;

/// NOLINTJUSTIFICATION global variables are only accessible from within this compilation unit
/// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
///
/// NOLINTJUSTIFICATION c array required to print a signal safe error message in memsetSigbusHandler
/// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
static char sigbusErrorMessage[SIGBUS_ERROR_MESSAGE_LENGTH];
static std::mutex sigbusHandlerMutex;
/// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

static void memsetSigbusHandler(int) noexcept
{
    auto result =
        write(STDERR_FILENO, &sigbusErrorMessage[0], strnlen(&sigbusErrorMessage[0], SIGBUS_ERROR_MESSAGE_LENGTH));
    IOX_DISCARD_RESULT(result);
    _exit(EXIT_FAILURE);
}

// NOLINTJUSTIFICATION the function size is related to the error handling and the cognitive complexity
// results from the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
cxx::expected<SharedMemoryObject, SharedMemoryObjectError> SharedMemoryObjectBuilder::create() noexcept
{
    auto printErrorDetails = [this] {
        LogError() << "Unable to create a shared memory object with the following properties [ name = " << m_name
                   << ", sizeInBytes = " << m_memorySizeInBytes << ", access mode = " << asStringLiteral(m_accessMode)
                   << ", open mode = " << asStringLiteral(m_openMode) << ", baseAddressHint = "
                   << ((m_baseAddressHint) ? iox::log::hex(m_baseAddressHint.value())
                                           : iox::log::hex(static_cast<uint64_t>(0U)))
                   << ((m_baseAddressHint) ? "" : " (no hint set)")
                   << ", permissions = " << iox::log::oct(static_cast<mode_t>(m_permissions)) << " ]";
    };

    auto sharedMemory = SharedMemoryBuilder()
                            .name(m_name)
                            .accessMode(m_accessMode)
                            .openMode(m_openMode)
                            .filePermissions(m_permissions)
                            .size(m_memorySizeInBytes)
                            .create();

    if (!sharedMemory)
    {
        printErrorDetails();
        LogError() << "Unable to create SharedMemoryObject since we could not acquire a SharedMemory resource";
        return cxx::error<SharedMemoryObjectError>(SharedMemoryObjectError::SHARED_MEMORY_CREATION_FAILED);
    }

    auto memoryMap = MemoryMapBuilder()
                         .baseAddressHint((m_baseAddressHint) ? *m_baseAddressHint : nullptr)
                         .length(m_memorySizeInBytes)
                         .fileDescriptor(sharedMemory->getHandle())
                         .accessMode(m_accessMode)
                         .flags(MemoryMapFlags::SHARE_CHANGES)
                         .offset(0)
                         .create();

    if (!memoryMap)
    {
        printErrorDetails();
        LogError() << "Failed to map created shared memory into process!";
        return cxx::error<SharedMemoryObjectError>(SharedMemoryObjectError::MAPPING_SHARED_MEMORY_FAILED);
    }

    Allocator allocator(memoryMap->getBaseAddress(), m_memorySizeInBytes);

    if (sharedMemory->hasOwnership())
    {
        LogDebug() << "Trying to reserve " << m_memorySizeInBytes << " bytes in the shared memory [" << m_name << "]";
        if (platform::IOX_SHM_WRITE_ZEROS_ON_CREATION)
        {
            // this lock is required for the case that multiple threads are creating multiple
            // shared memory objects concurrently
            std::lock_guard<std::mutex> lock(sigbusHandlerMutex);
            auto memsetSigbusGuard = registerSignalHandler(Signal::BUS, memsetSigbusHandler);
            if (memsetSigbusGuard.has_error())
            {
                printErrorDetails();
                LogError() << "Failed to temporarily override SIGBUS to safely zero the shared memory";
                return cxx::error<SharedMemoryObjectError>(SharedMemoryObjectError::INTERNAL_LOGIC_FAILURE);
            }

            // NOLINTJUSTIFICATION snprintf required to populate char array so that it can be used signal safe in
            //                     a possible signal call
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
            IOX_DISCARD_RESULT(snprintf(
                &sigbusErrorMessage[0],
                SIGBUS_ERROR_MESSAGE_LENGTH,
                "While setting the acquired shared memory to zero a fatal SIGBUS signal appeared caused by memset. The "
                "shared memory object with the following properties [ name = %s, sizeInBytes = %llu, access mode = %s, "
                "open mode = %s, baseAddressHint = %p, permissions = %lu ] maybe requires more memory than it is "
                "currently available in the system.\n",
                m_name.c_str(),
                static_cast<unsigned long long>(m_memorySizeInBytes),
                asStringLiteral(m_accessMode),
                asStringLiteral(m_openMode),
                (m_baseAddressHint) ? *m_baseAddressHint : nullptr,
                std::bitset<sizeof(mode_t)>(static_cast<mode_t>(m_permissions)).to_ulong()));

            memset(memoryMap->getBaseAddress(), 0, m_memorySizeInBytes);
        }
        LogDebug() << "Acquired " << m_memorySizeInBytes << " bytes successfully in the shared memory [" << m_name
                   << "]";
    }

    return cxx::success<SharedMemoryObject>(
        SharedMemoryObject(std::move(*sharedMemory), std::move(*memoryMap), std::move(allocator), m_memorySizeInBytes));
}

SharedMemoryObject::SharedMemoryObject(SharedMemory&& sharedMemory,
                                       MemoryMap&& memoryMap,
                                       Allocator&& allocator,
                                       const uint64_t memorySizeInBytes) noexcept
    : m_memorySizeInBytes(memorySizeInBytes)
    , m_sharedMemory(std::move(sharedMemory))
    , m_memoryMap(std::move(memoryMap))
    , m_allocator(std::move(allocator))
{
}

void* SharedMemoryObject::allocate(const uint64_t size, const uint64_t alignment) noexcept
{
    return m_allocator.allocate(size, alignment);
}

void SharedMemoryObject::finalizeAllocation() noexcept
{
    m_allocator.finalizeAllocation();
}

Allocator& SharedMemoryObject::getAllocator() noexcept
{
    return m_allocator;
}

const void* SharedMemoryObject::getBaseAddress() const noexcept
{
    return m_memoryMap.getBaseAddress();
}

void* SharedMemoryObject::getBaseAddress() noexcept
{
    return m_memoryMap.getBaseAddress();
}

uint64_t SharedMemoryObject::getSizeInBytes() const noexcept
{
    return m_memorySizeInBytes;
}

int32_t SharedMemoryObject::getFileHandle() const noexcept
{
    return m_sharedMemory.getHandle();
}

bool SharedMemoryObject::hasOwnership() const noexcept
{
    return m_sharedMemory.hasOwnership();
}


} // namespace posix
} // namespace iox
