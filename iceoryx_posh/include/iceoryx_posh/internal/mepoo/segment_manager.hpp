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

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/mepoo_segment.hpp"
#include "iceoryx_posh/mepoo/segment_config.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"

namespace iox
{
namespace roudi
{
template <typename MemoryManager, typename SegmentManager, typename SenderPort>
class MemPoolIntrospection;
}

namespace mepoo
{
template <typename SegmentType = MePooSegment<>>
class SegmentManager
{
  public:
    SegmentManager(const SegmentConfig& f_segmentConfig,
                   posix::Allocator* f_managementAllocator,
                   const uintptr_t f_sharedMemoryBaseAddressOffset,
                   const bool f_skipShmPlacementVerification = false);
    ~SegmentManager() = default;

    SegmentManager(const SegmentManager& rhs) = delete;
    SegmentManager(SegmentManager&& rhs) = delete;

    SegmentManager& operator=(const SegmentManager& rhs) = delete;
    SegmentManager& operator=(SegmentManager&& rhs) = delete;

    struct SegmentMapping
    {
      public:
        SegmentMapping(const std::string f_sharedMemoryName,
                       void* f_startAddress,
                       uint64_t f_size,
                       bool f_isWritable,
                       uint64_t f_segmentId)
            : m_sharedMemoryName(f_sharedMemoryName)
            , m_startAddress(f_startAddress)
            , m_size(f_size)
            , m_isWritable(f_isWritable)

            , m_segmentId(f_segmentId)

        {
        }

        std::string m_sharedMemoryName{""};
        void* m_startAddress{nullptr};
        uint64_t m_size{0};
        bool m_isWritable{false};
        uint64_t m_segmentId{0};
    };

    struct SegmentUserInformation
    {
        MemoryManager* m_memoryManager;
        uint64_t m_segmentID;
    };

    using SegmentMappingContainer = cxx::vector<SegmentMapping, MAX_SHM_SEGMENTS>;

    SegmentMappingContainer getSegmentMappings(posix::PosixUser f_user);
    SegmentUserInformation getSegmentInformationForUser(posix::PosixUser f_user);

    static uint64_t requiredManagementMemorySize(const RouDiConfig_t& f_config);
    static uint64_t requiredChunkMemorySize(const RouDiConfig_t& f_config);
    static uint64_t requiredFullMemorySize(const RouDiConfig_t& f_config);

  private:
    bool createSegment(const SegmentConfig::SegmentEntry& f_segmentEntry);

  private:
    template <typename MemoryManger, typename SegmentManager, typename SenderPort>
    friend class roudi::MemPoolIntrospection;

    uintptr_t m_nextSegmentBaseAddressOffset{0};
    posix::Allocator* m_managementAllocator;
    cxx::vector<SegmentType, MAX_SHM_SEGMENTS> m_segmentContainer;
    bool m_createInterfaceEnabled{true};
    bool m_verifySharedMemoryPlacement{false};
};


} // namespace mepoo
} // namespace iox

#include "iceoryx_posh/internal/mepoo/segment_manager.inl"
