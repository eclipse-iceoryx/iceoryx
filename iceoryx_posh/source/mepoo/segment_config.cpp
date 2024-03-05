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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/mepoo/segment_config.hpp"
#include "iox/posix_group.hpp"

namespace iox
{
namespace mepoo
{
SegmentConfig& SegmentConfig::setDefaults() noexcept
{
    m_sharedMemorySegments.clear();
    auto groupName = PosixGroup::getGroupOfCurrentProcess().getName();
    m_sharedMemorySegments.push_back({groupName, groupName, MePooConfig().setDefaults()});
    return *this;
}

SegmentConfig& SegmentConfig::optimize() noexcept
{
    /* this might be dangerous, if two segments with similar chunk sizes are merged, only the chunks from smaller
    mempool might be used auto segmentConfig = m_sharedMemorySegments; m_sharedMemorySegments.clear(); for (auto&
    currentSegmentEntry : segmentConfig)
    {
        if (!currentSegmentEntry.m_mempoolConfig.m_mempoolConfig.empty())
        {
            SegmentConfig::SegmentEntry segmentEntry;
            segmentEntry.m_readerGroup = currentSegmentEntry.m_readerGroup;
            segmentEntry.m_writerGroup = currentSegmentEntry.m_writerGroup;
            for (auto& currentSegment : segmentConfig)
            {
                if (currentSegment.m_readerGroup == segmentEntry.m_readerGroup
                    && currentSegment.m_writerGroup == segmentEntry.m_writerGroup)
                {
                    for (const auto& mempoolEntry : currentSegment.m_mempoolConfig.m_mempoolConfig)
                    {
                        segmentEntry.m_mempoolConfig.m_mempoolConfig.push_back(mempoolEntry);
                    }
                    currentSegment.m_mempoolConfig.m_mempoolConfig.clear();
                }
            }

            segmentEntry.m_mempoolConfig.optimize();

            m_sharedMemorySegments.push_back(segmentEntry);
        }
    }
    */
    return *this;
}
} // namespace mepoo
} // namespace iox
