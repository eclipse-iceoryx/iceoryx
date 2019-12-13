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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/system_configuration.hpp"

namespace iox
{
namespace mepoo
{
template <typename SegmentType>
inline SegmentManager<SegmentType>::SegmentManager(const SegmentConfig& f_segmentConfig,
                                                   posix::Allocator* f_managementAllocator,
                                                   const uintptr_t f_sharedMemoryBaseAddressOffset,
                                                   const bool f_verifySharedMemoryPlacement)
    : m_nextSegmentBaseAddressOffset(f_sharedMemoryBaseAddressOffset)
    , m_managementAllocator(f_managementAllocator)
    , m_verifySharedMemoryPlacement(f_verifySharedMemoryPlacement)
{
    for (const auto& segmentEntry : f_segmentConfig.m_sharedMemorySegments)
    {
        createSegment(segmentEntry);
    }
}

template <typename SegmentType>
inline bool SegmentManager<SegmentType>::createSegment(const SegmentConfig::SegmentEntry& f_segmentEntry)
{
    if (m_segmentContainer.size() < m_segmentContainer.capacity())
    {
        auto readerGroup = iox::posix::PosixGroup(f_segmentEntry.m_readerGroup);
        auto writerGroup = iox::posix::PosixGroup(f_segmentEntry.m_writerGroup);
        m_segmentContainer.emplace_back(f_segmentEntry.m_mempoolConfig,
                                        m_managementAllocator,
                                        readerGroup,
                                        writerGroup,
                                        m_nextSegmentBaseAddressOffset);

        const auto& segment = m_segmentContainer.back();

        if (m_verifySharedMemoryPlacement
            && reinterpret_cast<uintptr_t>(segment.getSharedMemoryObject().getBaseAddress())
                   != m_nextSegmentBaseAddressOffset)
        {
            errorHandler(Error::kMEPOO__SEGMENT_BASEADDRESS_VIOLATES_SPECIFICATION);
        }

        m_nextSegmentBaseAddressOffset += cxx::align(segment.getSharedMemoryObject().getSizeInBytes(),
                                                     posix::pageSize().value_or(posix::MaxPageSize));

        return true;
    }
    else
    {
        errorHandler(Error::kMEPOO__SEGMENT_CONTAINER_OVERFLOW);
        return false;
    }
}

template <typename SegmentType>
inline typename SegmentManager<SegmentType>::SegmentMappingContainer
SegmentManager<SegmentType>::getSegmentMappings(posix::PosixUser f_user)
{
    // get all the groups the user is in
    auto l_groupContainer = f_user.getGroups();

    SegmentManager::SegmentMappingContainer l_mappingContainer;
    bool l_foundInWriterGroup = false;

    // with the groups we can get all the segments (read or write) for the user
    for (const auto& groupID : l_groupContainer)
    {
        for (const auto& segment : m_segmentContainer)
        {
            if (segment.getWriterGroup() == groupID)
            {
                // a user is allowed to be only in one writer group, as we currently only support one memory manager per
                // process
                if (!l_foundInWriterGroup)
                {
                    l_mappingContainer.emplace_back("/" + segment.getWriterGroup().getName(),
                                                    segment.getSharedMemoryObject().getBaseAddress(),
                                                    segment.getSharedMemoryObject().getSizeInBytes(),
                                                    true,
                                                    segment.getSegmentId());
                    l_foundInWriterGroup = true;
                }
                else
                {
                    errorHandler(Error::kMEPOO__USER_WITH_MORE_THAN_ONE_WRITE_SEGMENT);
                }
            }
        }
    }

    for (const auto& groupID : l_groupContainer)
    {
        for (const auto& segment : m_segmentContainer)
        {
            // only add segments which are not yet added as writer
            if (segment.getReaderGroup() == groupID
                && std::find_if(l_mappingContainer.begin(),
                                l_mappingContainer.end(),
                                [&](const SegmentMapping& mapping) {
                                    return mapping.m_startAddress == segment.getSharedMemoryObject().getBaseAddress();
                                })
                       == l_mappingContainer.end())
            {
                l_mappingContainer.emplace_back("/" + segment.getWriterGroup().getName(),
                                                segment.getSharedMemoryObject().getBaseAddress(),
                                                segment.getSharedMemoryObject().getSizeInBytes(),
                                                false,
                                                segment.getSegmentId());
            }
        }
    }

    return l_mappingContainer;
}

template <typename SegmentType>
inline typename SegmentManager<SegmentType>::SegmentUserInformation
SegmentManager<SegmentType>::getSegmentInformationForUser(posix::PosixUser f_user)
{
    auto l_groupContainer = f_user.getGroups();

    SegmentUserInformation segmentInfo{nullptr, 0u};

    // with the groups we can search for the writable segment of this user
    for (const auto& groupID : l_groupContainer)
    {
        for (auto& segment : m_segmentContainer)
        {
            if (segment.getWriterGroup() == groupID)
            {
                segmentInfo.m_memoryManager = &segment.getMemoryManager();
                segmentInfo.m_segmentID = segment.getSegmentId();
                return segmentInfo;
            }
        }
    }

    return segmentInfo;
}

template <typename SegmentType>
uint64_t SegmentManager<SegmentType>::requiredManagementMemorySize(const RouDiConfig_t& f_config)
{
    uint64_t memorySize{0};
    for (auto segment : f_config.m_sharedMemorySegments)
    {
        memorySize += cxx::align(MemoryManager::requiredManagementMemorySize(segment.m_mempoolConfig), 32ul);
    }
    return memorySize;
}

template <typename SegmentType>
uint64_t SegmentManager<SegmentType>::requiredChunkMemorySize(const RouDiConfig_t& f_config)
{
    uint64_t memorySize{0};
    for (auto segment : f_config.m_sharedMemorySegments)
    {
        memorySize += cxx::align(MemoryManager::requiredChunkMemorySize(segment.m_mempoolConfig), 32ul);
    }
    return memorySize;
}

template <typename SegmentType>
uint64_t SegmentManager<SegmentType>::requiredFullMemorySize(const RouDiConfig_t& f_config)
{
    return cxx::align(requiredManagementMemorySize(f_config) + requiredChunkMemorySize(f_config), 32ul);
}

} // namespace mepoo
} // namespace iox
