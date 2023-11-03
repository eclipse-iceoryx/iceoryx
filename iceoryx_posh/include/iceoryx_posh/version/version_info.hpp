// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_VERSION_VERSION_INFO_HPP
#define IOX_POSH_VERSION_VERSION_INFO_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iceoryx_versions.hpp"
#include "iox/detail/serialization.hpp"
#include "iox/size.hpp"
#include "iox/string.hpp"

#include <cstdint>

namespace iox
{
namespace version
{
/// @brief Is used to compare the RouDis and runtime's version information.
class VersionInfo
{
  public:
    /// @brief Generates an VersionInfo initialized with the information given by the auto generated
    /// iceoryx_versions.hpp defines.
    /// @param{in] versionMajor The major version.
    /// @param{in] versionMinor The minor version.
    /// @param{in] versionPatch The patch version.
    /// @param{in] versionTweak The tweak/RC version.
    /// @param{in] buildDateString The date when the component was build as string with maximal 36 readable chars.
    /// @param{in] commitIdString The commit id is shortened internally to 12 readable chars.
    VersionInfo(const uint16_t versionMajor,
                const uint16_t versionMinor,
                const uint16_t versionPatch,
                const uint16_t versionTweak,
                const BuildDateString_t& buildDateString,
                const CommitIdString_t& commitIdString) noexcept;

    /// @brief Construction of the VersionInfo using serialized strings.
    /// @param[in] serial The serialization object from read from to initialize this object.
    VersionInfo(const Serialization& serial) noexcept;

    /// @brief Serialization of the VersionInfo.
    operator Serialization() const noexcept;

    /// @brief Checks if the to versions information are identical.
    /// @param[in] rhs The right side of the compare with equal.
    bool operator==(const VersionInfo& rhs) const noexcept;

    /// @brief Checks if the to versions information are not identical.
    /// @param[in] rhs The right side of the compare with unequal.
    bool operator!=(const VersionInfo& rhs) const noexcept;

    /// @brief Compares this version versus another with respect to the compatibility value give.
    /// @param[in] other The other version compared with this version.
    /// @param[in] compatibilityCheckLevel Gives the level how deep it should be compared.
    bool checkCompatibility(const VersionInfo& other,
                            const CompatibilityCheckLevel compatibilityCheckLevel) const noexcept;

    /// @brief The serialization could fail, which cause an invalid object, else true.
    /// @return Returns if the object is valid.
    bool isValid() noexcept;

    /// @brief Creates a version object of the current iceoryx version.
    /// @return Returns the current version of iceoryx as an object.
    static VersionInfo getCurrentVersion() noexcept;

  protected:
    static constexpr uint64_t SERIALIZATION_STRING_SIZE = 100;
    using SerializationString_t = string<SERIALIZATION_STRING_SIZE>;

    static_assert(COMMIT_ID_STRING_SIZE <= SERIALIZATION_STRING_SIZE, "CommitId needs to transfered completely.");
    static_assert(COMMIT_ID_STRING_SIZE <= BUILD_DATE_STRING_SIZE, "BuildDate needs to transfered completely.");
    static constexpr uint64_t NULL_TERMINATION_SIZE{1};
    static_assert((size(ICEORYX_BUILDDATE) - NULL_TERMINATION_SIZE) < BUILD_DATE_STRING_SIZE,
                  "COMMIT_BUILD_DATE_STRING_SIZE needs to be bigger.");

  protected:
    bool m_valid{true};
    uint16_t m_versionMajor{0};
    uint16_t m_versionMinor{0};
    uint16_t m_versionPatch{0};
    uint16_t m_versionTweak{0};
    BuildDateString_t m_buildDateString;
    CommitIdString_t m_commitIdString;
};

} // namespace version
} // namespace iox
#endif // IOX_POSH_VERSION_VERSION_INFO_HPP
