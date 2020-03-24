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

#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <string>

namespace iox
{
namespace mepoo
{
struct SegmentConfig
{
    struct SegmentEntry
    {
        SegmentEntry(const std::string& readerGroup,
                     const std::string& writerGroup,
                     const MePooConfig& memPoolConfig,
                     iox::mepoo::MemoryInfo memoryInfo = iox::mepoo::MemoryInfo())
            : m_readerGroup(readerGroup)
            , m_writerGroup(writerGroup)
            , m_mempoolConfig(memPoolConfig)
            , m_memoryInfo(memoryInfo)

        {
        }

        std::string m_readerGroup;
        std::string m_writerGroup;
        MePooConfig m_mempoolConfig;
        iox::mepoo::MemoryInfo m_memoryInfo;
    };

    cxx::vector<SegmentEntry, MAX_SHM_SEGMENTS> m_sharedMemorySegments;

    /// @brief Set Function for default values to be added in SegmentConfig
    SegmentConfig& setDefaults() noexcept;

    SegmentConfig& optimize() noexcept;
};
} // namespace mepoo
} // namespace iox

