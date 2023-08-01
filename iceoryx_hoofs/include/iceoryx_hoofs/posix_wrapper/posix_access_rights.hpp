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
#ifndef IOX_HOOFS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP
#define IOX_HOOFS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP

#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_platform/types.hpp"
#include "iox/optional.hpp"
#include "iox/string.hpp"
#include "iox/vector.hpp"

#include <string>

namespace iox
{
namespace posix
{
static constexpr int MaxNumberOfGroups = 888;

class PosixGroup
{
  public:
    using groupName_t = string<platform::MAX_GROUP_NAME_LENGTH>;
    explicit PosixGroup(const gid_t id) noexcept;
    explicit PosixGroup(const groupName_t& name) noexcept;

    bool operator==(const PosixGroup& other) const noexcept;

    groupName_t getName() const noexcept;
    gid_t getID() const noexcept;

    bool doesExist() const noexcept;

    static PosixGroup getGroupOfCurrentProcess() noexcept;

    static optional<uid_t> getGroupID(const groupName_t& name) noexcept;
    static optional<groupName_t> getGroupName(gid_t id) noexcept;

  private:
    gid_t m_id;
    bool m_doesExist{false};
};

class PosixUser
{
  public:
    using groupVector_t = vector<PosixGroup, MaxNumberOfGroups>;

    using userName_t = string<platform::MAX_USER_NAME_LENGTH>;

    explicit PosixUser(const uid_t id) noexcept;
    explicit PosixUser(const userName_t& name) noexcept;

    groupVector_t getGroups() const noexcept;
    userName_t getName() const noexcept;
    uid_t getID() const noexcept;

    bool doesExist() const noexcept;

    static PosixUser getUserOfCurrentProcess() noexcept;

    static optional<uid_t> getUserID(const userName_t& name) noexcept;
    static optional<userName_t> getUserName(uid_t id) noexcept;

  private:
    uid_t m_id;
    bool m_doesExist{false};
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP
