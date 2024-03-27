// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 - 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/mepoo_segment.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/detail/convert.hpp"
#include "iox/logging.hpp"
#include "iox/relative_pointer.hpp"

namespace iox
{
namespace mepoo
{
template <typename SharedMemoryObjectType, typename MemoryManagerType>
constexpr access_rights MePooSegment<SharedMemoryObjectType, MemoryManagerType>::SEGMENT_PERMISSIONS;

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline MePooSegment<SharedMemoryObjectType, MemoryManagerType>::MePooSegment(
    const MePooConfig& mempoolConfig,
    const DomainId domainId,
    BumpAllocator& managementAllocator,
    const PosixGroup& readerGroup,
    const PosixGroup& writerGroup,
    const iox::mepoo::MemoryInfo& memoryInfo) noexcept
    : m_readerGroup(readerGroup)
    , m_writerGroup(writerGroup)
    , m_memoryInfo(memoryInfo)
    , m_sharedMemoryObject(createSharedMemoryObject(mempoolConfig, domainId, writerGroup))
{
    using namespace detail;
    PosixAcl acl;
    if (!(readerGroup == writerGroup))
    {
        acl.addGroupPermission(PosixAcl::Permission::READ, readerGroup.getName());
    }
    acl.addGroupPermission(PosixAcl::Permission::READWRITE, writerGroup.getName());
    acl.addPermissionEntry(PosixAcl::Category::USER, PosixAcl::Permission::READWRITE);
    acl.addPermissionEntry(PosixAcl::Category::GROUP, PosixAcl::Permission::READWRITE);
    acl.addPermissionEntry(PosixAcl::Category::OTHERS, PosixAcl::Permission::NONE);

    if (!acl.writePermissionsToFile(m_sharedMemoryObject.getFileHandle()))
    {
        IOX_REPORT_FATAL(PoshError::MEPOO__SEGMENT_COULD_NOT_APPLY_POSIX_RIGHTS_TO_SHARED_MEMORY);
    }

    BumpAllocator allocator(m_sharedMemoryObject.getBaseAddress(),
                            m_sharedMemoryObject.get_size().expect("Failed to get SHM size."));
    m_memoryManager.configureMemoryManager(mempoolConfig, managementAllocator, allocator);
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline SharedMemoryObjectType MePooSegment<SharedMemoryObjectType, MemoryManagerType>::createSharedMemoryObject(
    const MePooConfig& mempoolConfig, const DomainId domainId, const PosixGroup& writerGroup) noexcept
{
    return std::move(
        typename SharedMemoryObjectType::Builder()
            .name([&domainId, &writerGroup] {
                using ShmName_t = detail::PosixSharedMemory::Name_t;
                ShmName_t shmName = iceoryxResourcePrefix(domainId, ResourceType::USER_DEFINED);
                if (shmName.size() + writerGroup.getName().size() > ShmName_t::capacity())
                {
                    IOX_LOG(FATAL,
                            "The payload segment with the name '"
                                << writerGroup.getName().size()
                                << "' would exceed the maximum allowed size when used with the '" << shmName
                                << "' prefix!");
                    IOX_PANIC("");
                }
                shmName.append(TruncateToCapacity, writerGroup.getName());
                return shmName;
            }())
            .memorySizeInBytes(MemoryManager::requiredChunkMemorySize(mempoolConfig))
            .accessMode(AccessMode::READ_WRITE)
            .openMode(OpenMode::PURGE_AND_CREATE)
            .permissions(SEGMENT_PERMISSIONS)
            .create()
            .and_then([this](auto& sharedMemoryObject) {
                auto maybeSegmentId = iox::UntypedRelativePointer::registerPtr(
                    sharedMemoryObject.getBaseAddress(),
                    sharedMemoryObject.get_size().expect("Failed to get SHM size"));
                if (!maybeSegmentId.has_value())
                {
                    IOX_REPORT_FATAL(PoshError::MEPOO__SEGMENT_INSUFFICIENT_SEGMENT_IDS);
                }
                this->m_segmentId = static_cast<uint64_t>(maybeSegmentId.value());
                this->m_segmentSize = sharedMemoryObject.get_size().expect("Failed to get SHM size.");

                IOX_LOG(DEBUG,
                        "Roudi registered payload data segment " << iox::log::hex(sharedMemoryObject.getBaseAddress())
                                                                 << " with size " << m_segmentSize << " to id "
                                                                 << m_segmentId);
            })
            .or_else([](auto&) { IOX_REPORT_FATAL(PoshError::MEPOO__SEGMENT_UNABLE_TO_CREATE_SHARED_MEMORY_OBJECT); })
            .value());
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline PosixGroup MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getWriterGroup() const noexcept
{
    return m_writerGroup;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline PosixGroup MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getReaderGroup() const noexcept
{
    return m_readerGroup;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline MemoryManagerType& MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getMemoryManager() noexcept
{
    return m_memoryManager;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline uint64_t MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getSegmentId() const noexcept
{
    return m_segmentId;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline uint64_t MePooSegment<SharedMemoryObjectType, MemoryManagerType>::getSegmentSize() const noexcept
{
    return m_segmentSize;
}

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_MEPOO_SEGMENT_INL
