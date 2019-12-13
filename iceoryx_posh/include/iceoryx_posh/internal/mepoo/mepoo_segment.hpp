// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/access_control.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"

namespace iox
{
namespace mepoo
{
template <typename SharedMemoryObjectType = posix::SharedMemoryObject, typename MemoryManagerType = MemoryManager>
class MePooSegment
{
  public:
    MePooSegment(const MePooConfig& f_mempoolConfig,
                 posix::Allocator* f_managementAllocator,
                 const posix::PosixGroup& f_readerGroup,
                 const posix::PosixGroup& f_writerGroup,
                 const uintptr_t f_baseAddressOffset);

    posix::PosixGroup getWriterGroup() const;
    posix::PosixGroup getReaderGroup() const;
    const SharedMemoryObjectType& getSharedMemoryObject() const;
    MemoryManagerType& getMemoryManager();

    uint64_t getSegmentId() const;

  protected:
    SharedMemoryObjectType createSharedMemoryObject(const MePooConfig& f_mempoolConfig,
                                                    const posix::PosixGroup& f_writerGroup,
                                                    const uintptr_t f_baseAddressOffset);

  protected:
    SharedMemoryObjectType m_sharedMemoryObject;
    MemoryManagerType m_memoryManager;
    posix::PosixGroup m_readerGroup;
    posix::PosixGroup m_writerGroup;
    uint64_t m_segmentId;

  private:
    void setSegmentId(const uint64_t segmentId);
};
} // namespace mepoo
} // namespace iox

#include "iceoryx_posh/internal/mepoo/mepoo_segment.inl"
