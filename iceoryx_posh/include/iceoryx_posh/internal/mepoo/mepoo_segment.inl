// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_MEPOO_SEGMENT_INL
#define IOX_POSH_MEPOO_MEPOO_SEGMENT_INL

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
namespace mepoo
{
template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline MePooSegment<SharedMemoryObjectType, MemoryManagerType>::MePooSegment(const MePooConfig& f_mempoolConfig,
                                                                             posix::Allocator* f_managementAllocator,
                                                                             const posix::PosixGroup& f_readerGroup,
                                                                             const posix::PosixGroup& f_writerGroup,
                                                                             const iox::mepoo::MemoryInfo& memoryInfo)
    : m_sharedMemoryObject(std::move(createSharedMemoryObject(f_mempoolConfig, f_writerGroup)))
    , m_readerGroup(f_readerGroup)
    , m_writerGroup(f_writerGroup)
    , m_memoryInfo(memoryInfo)
{
    using namespace posix;
    AccessController f_accessController;
    if (!(f_readerGroup == f_writerGroup))
    {
        f_accessController.addPermissionEntry(
            AccessController::Category::SPECIFIC_GROUP, AccessController::Permission::READ, f_readerGroup.getName());
    }
    f_accessController.addPermissionEntry(
        AccessController::Category::SPECIFIC_GROUP, AccessController::Permission::READWRITE, f_writerGroup.getName());
    f_accessController.addPermissionEntry(AccessController::Category::USER, AccessController::Permission::READWRITE);
    f_accessController.addPermissionEntry(AccessController::Category::GROUP, AccessController::Permission::READWRITE);
    f_accessController.addPermissionEntry(AccessController::Category::OTHERS, AccessController::Permission::NONE);

    if (!f_accessController.writePermissionsToFile(m_sharedMemoryObject.getFileHandle()))
    {
        errorHandler(Error::kMEPOO__SEGMENT_COULD_NOT_APPLY_POSIX_RIGHTS_TO_SHARED_MEMORY);
    }

    m_memoryManager.configureMemoryManager(f_mempoolConfig, f_managementAllocator, m_sharedMemoryObject.getAllocator());
    m_sharedMemoryObject.finalizeAllocation();
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline SharedMemoryObjectType MePooSegment<SharedMemoryObjectType, MemoryManagerType>::createSharedMemoryObject(
    const MePooConfig& f_mempoolConfig, const posix::PosixGroup& f_writerGroup)
{
    // we let the OS decide where to map the shm segments
    constexpr void* BASE_ADDRESS_HINT{nullptr};

    // on qnx the current working directory will be added to the /dev/shmem path if the leading slash is missing
    posix::SharedMemory::Name_t shmName = "/" + f_writerGroup.getName();

    return std::move(
        SharedMemoryObjectType::create(shmName,
                                       MemoryManager::requiredChunkMemorySize(f_mempoolConfig),
                                       posix::AccessMode::READ_WRITE,
                                       posix::OwnerShip::MINE,
                                       BASE_ADDRESS_HINT,
                                       static_cast<mode_t>(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
            .and_then([this](auto& sharedMemoryObject) {
                setSegmentId(iox::RelativePointer::registerPtr(sharedMemoryObject.getBaseAddress(),
                                                               sharedMemoryObject.getSizeInBytes()));

                LogDebug() << "Roudi registered payload segment "
                           << iox::log::HexFormat(reinterpret_cast<uint64_t>(sharedMemoryObject.getBaseAddress()))
                           << " with size " << sharedMemoryObject.getSizeInBytes() << " to id " << m_segmentId;
            })
            .or_else([](auto&) { errorHandler(Error::kMEPOO__SEGMENT_UNABLE_TO_CREATE_SHARED_MEMORY_OBJECT); })
            .value());
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline posix::PosixGroup MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getWriterGroup() const
{
    return m_writerGroup;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline posix::PosixGroup MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getReaderGroup() const
{
    return m_readerGroup;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline MemoryManagerType& MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getMemoryManager()
{
    return m_memoryManager;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline const SharedMemoryObjectType&
MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getSharedMemoryObject() const
{
    return m_sharedMemoryObject;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline uint64_t MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getSegmentId() const
{
    return m_segmentId;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline void MePooSegment<SharedMemoryObjectType, MemoryManagerType>::setSegmentId(const uint64_t segmentId)
{
    m_segmentId = segmentId;
}

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_MEPOO_SEGMENT_INL
