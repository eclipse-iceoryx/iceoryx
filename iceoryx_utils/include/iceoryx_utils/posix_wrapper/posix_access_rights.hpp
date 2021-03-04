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
#ifndef IOX_UTILS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP
#define IOX_UTILS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/platform/types.hpp"

#include <string>

namespace iox
{
namespace posix
{
static constexpr int MaxNumberOfGroups = 888;

struct PosixRights
{
    PosixRights(bool f_read, bool f_write, bool f_execute);
    bool m_read;
    bool m_write;
    bool m_execute;
};

class PosixGroup
{
  public:
    using string_t = cxx::string<100>;
    PosixGroup(gid_t f_id);
    PosixGroup(string_t f_name);

    bool operator==(const PosixGroup& other) const;

    string_t getName() const;
    gid_t getID() const;

    bool doesExist() const;

    static PosixGroup getGroupOfCurrentProcess();

    static cxx::optional<uid_t> getGroupID(const string_t& f_name);
    static cxx::optional<string_t> getGroupName(gid_t f_id);

  private:
    gid_t m_id;
    bool m_doesExist{false};
};

class PosixUser
{
  public:
    using groupVector_t = cxx::vector<PosixGroup, MaxNumberOfGroups>;
    using string_t = cxx::string<100>;

    PosixUser(uid_t f_id);
    PosixUser(const string_t& f_name);

    groupVector_t getGroups() const;
    string_t getName() const;
    uid_t getID() const;

    bool doesExist() const;

    static PosixUser getUserOfCurrentProcess();

    static cxx::optional<uid_t> getUserID(const string_t& f_name);
    static cxx::optional<string_t> getUserName(uid_t f_id);

  private:
    uid_t m_id;
    bool m_doesExist{false};
};

} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP
