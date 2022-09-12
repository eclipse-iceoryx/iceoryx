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

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_platform/types.hpp"

#include <string>

namespace iox
{
namespace posix
{
static constexpr int MaxNumberOfGroups = 888;

class PosixGroup
{
  public:
    static constexpr uint64_t MAX_GROUP_NAME_LENGTH = 16;
    using groupName_t = cxx::string<MAX_GROUP_NAME_LENGTH>;
    explicit PosixGroup(const gid_t id) noexcept;
    explicit PosixGroup(const groupName_t& name) noexcept;

    bool operator==(const PosixGroup& other) const noexcept;

    groupName_t getName() const noexcept;
    gid_t getID() const noexcept;

    bool doesExist() const noexcept;

    static PosixGroup getGroupOfCurrentProcess() noexcept;

    static cxx::optional<uid_t> getGroupID(const groupName_t& name) noexcept;
    static cxx::optional<groupName_t> getGroupName(gid_t id) noexcept;

  private:
    gid_t m_id;
    bool m_doesExist{false};
};

class PosixUser
{
  public:
    using groupVector_t = cxx::vector<PosixGroup, MaxNumberOfGroups>;

    static constexpr uint64_t MAX_USER_NAME_LENGTH = 32;
    using userName_t = cxx::string<MAX_USER_NAME_LENGTH>;

    explicit PosixUser(const uid_t id) noexcept;
    explicit PosixUser(const userName_t& name) noexcept;

    groupVector_t getGroups() const noexcept;
    userName_t getName() const noexcept;
    uid_t getID() const noexcept;

    bool doesExist() const noexcept;

    static PosixUser getUserOfCurrentProcess() noexcept;

    static cxx::optional<uid_t> getUserID(const userName_t& name) noexcept;
    static cxx::optional<userName_t> getUserName(uid_t id) noexcept;

  private:
    uid_t m_id;
    bool m_doesExist{false};
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP
