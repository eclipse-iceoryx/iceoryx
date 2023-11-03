// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/version/version_info.hpp"

#include <algorithm>

namespace iox
{
namespace version
{
VersionInfo::VersionInfo(const uint16_t versionMajor,
                         const uint16_t versionMinor,
                         const uint16_t versionPatch,
                         const uint16_t versionTweak,
                         const BuildDateString_t& buildDateString,
                         const CommitIdString_t& commitIdString) noexcept
    : m_versionMajor(versionMajor)
    , m_versionMinor(versionMinor)
    , m_versionPatch(versionPatch)
    , m_versionTweak(versionTweak)
    , m_buildDateString(buildDateString)
    , m_commitIdString(commitIdString)
{
}

VersionInfo::VersionInfo(const Serialization& serial) noexcept
{
    SerializationString_t tmp_m_buildDateString;
    SerializationString_t tmp_commitIdString;
    m_valid = serial.extract(
        m_versionMajor, m_versionMinor, m_versionPatch, m_versionTweak, tmp_m_buildDateString, tmp_commitIdString);
    m_buildDateString = BuildDateString_t(TruncateToCapacity, tmp_m_buildDateString.c_str());
    m_commitIdString = CommitIdString_t(TruncateToCapacity, tmp_commitIdString.c_str());
}

/// @brief Serialization of the VersionInfo.
VersionInfo::operator Serialization() const noexcept
{
    SerializationString_t tmp_m_buildDateString = m_buildDateString;
    SerializationString_t tmp_commitIdString = m_commitIdString;
    return Serialization::create(
        m_versionMajor, m_versionMinor, m_versionPatch, m_versionTweak, tmp_m_buildDateString, tmp_commitIdString);
}

bool VersionInfo::operator==(const VersionInfo& rhs) const noexcept
{
    return (m_valid == rhs.m_valid) && (m_versionMajor == rhs.m_versionMajor) && (m_versionMinor == rhs.m_versionMinor)
           && (m_versionPatch == rhs.m_versionPatch) && (m_versionTweak == rhs.m_versionTweak)
           && (m_buildDateString == rhs.m_buildDateString) && (m_commitIdString == rhs.m_commitIdString);
}

bool VersionInfo::operator!=(const VersionInfo& rhs) const noexcept
{
    return !(*this == rhs);
}

bool VersionInfo::checkCompatibility(const VersionInfo& other,
                                     const CompatibilityCheckLevel compatibilityCheckLevel) const noexcept
{
    bool isCompatible = false;
    switch (compatibilityCheckLevel)
    {
    case CompatibilityCheckLevel::OFF:
        isCompatible = true;
        break;
    case CompatibilityCheckLevel::MAJOR:
        isCompatible = (m_valid == other.m_valid) && (m_versionMajor == other.m_versionMajor);
        break;
    case CompatibilityCheckLevel::MINOR:
        isCompatible = (m_valid == other.m_valid) && (m_versionMajor == other.m_versionMajor)
                       && (m_versionMinor == other.m_versionMinor);
        break;
    case CompatibilityCheckLevel::PATCH:
        isCompatible = (m_valid == other.m_valid) && (m_versionMajor == other.m_versionMajor)
                       && (m_versionMinor == other.m_versionMinor) && (m_versionPatch == other.m_versionPatch);
        break;
    case CompatibilityCheckLevel::COMMIT_ID:
        isCompatible = (m_valid == other.m_valid) && (m_versionMajor == other.m_versionMajor)
                       && (m_versionMinor == other.m_versionMinor) && (m_versionPatch == other.m_versionPatch)
                       && (m_versionTweak == other.m_versionTweak) && (m_commitIdString == other.m_commitIdString);
        break;
    case CompatibilityCheckLevel::BUILD_DATE:
        isCompatible = (*this == other);
        break;
    }
    return isCompatible;
}

bool VersionInfo::isValid() noexcept
{
    return m_valid;
}

VersionInfo VersionInfo::getCurrentVersion() noexcept
{
    BuildDateString_t buildDateStringCxx(ICEORYX_BUILDDATE);
    CommitIdString_t shortCommitIdString(TruncateToCapacity, ICEORYX_SHA1, COMMIT_ID_STRING_SIZE);

    return VersionInfo(static_cast<uint16_t>(ICEORYX_VERSION_MAJOR),
                       static_cast<uint16_t>(ICEORYX_VERSION_MINOR),
                       static_cast<uint16_t>(ICEORYX_VERSION_PATCH),
                       static_cast<uint16_t>(ICEORYX_VERSION_TWEAK),
                       buildDateStringCxx,
                       shortCommitIdString);
}

} // namespace version
} // namespace iox
