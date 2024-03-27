// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_MEPOO_SEGMENT_HPP
#define IOX_POSH_MEPOO_MEPOO_SEGMENT_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/detail/posix_acl.hpp"
#include "iox/filesystem.hpp"
#include "iox/posix_group.hpp"
#include "iox/posix_shared_memory_object.hpp"

namespace iox
{
namespace mepoo
{
template <typename SharedMemoryObjectType = PosixSharedMemoryObject, typename MemoryManagerType = MemoryManager>
class MePooSegment
{
  public:
    MePooSegment(const MePooConfig& mempoolConfig,
                 const DomainId domainId,
                 BumpAllocator& managementAllocator,
                 const PosixGroup& readerGroup,
                 const PosixGroup& writerGroup,
                 const iox::mepoo::MemoryInfo& memoryInfo = iox::mepoo::MemoryInfo()) noexcept;

    PosixGroup getWriterGroup() const noexcept;
    PosixGroup getReaderGroup() const noexcept;

    MemoryManagerType& getMemoryManager() noexcept;

    uint64_t getSegmentId() const noexcept;

    uint64_t getSegmentSize() const noexcept;

  protected:
    SharedMemoryObjectType createSharedMemoryObject(const MePooConfig& mempoolConfig,
                                                    const DomainId domainId,
                                                    const PosixGroup& writerGroup) noexcept;

  protected:
    PosixGroup m_readerGroup;
    PosixGroup m_writerGroup;
    uint64_t m_segmentId{0};
    uint64_t m_segmentSize{0};
    iox::mepoo::MemoryInfo m_memoryInfo;
    SharedMemoryObjectType m_sharedMemoryObject;
    MemoryManagerType m_memoryManager;

    static constexpr access_rights SEGMENT_PERMISSIONS =
        perms::owner_read | perms::owner_write | perms::group_read | perms::group_write;
};
} // namespace mepoo
} // namespace iox

#include "iceoryx_posh/internal/mepoo/mepoo_segment.inl"

#endif // IOX_POSH_MEPOO_MEPOO_SEGMENT_HPP
