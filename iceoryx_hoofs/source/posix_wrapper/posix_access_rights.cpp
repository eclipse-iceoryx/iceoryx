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

#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"

#include "iceoryx_hoofs/platform/grp.hpp"
#include "iceoryx_hoofs/platform/platform_correction.hpp"
#include "iceoryx_hoofs/platform/pwd.hpp"
#include "iceoryx_hoofs/platform/types.hpp"
#include "iceoryx_hoofs/platform/unistd.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <limits>

namespace iox
{
namespace posix
{
PosixRights::PosixRights(bool f_read, bool f_write, bool f_execute) noexcept
    : m_read(f_read)
    , m_write(f_write)
    , m_execute(f_execute)
{
}

PosixGroup::PosixGroup(gid_t f_id) noexcept
    : m_id(f_id)
    , m_doesExist(getGroupName(f_id).has_value())
{
}

PosixGroup::PosixGroup(const PosixGroup::string_t& f_name) noexcept
{
    auto id = getGroupID(f_name);
    if (id.has_value())
    {
        m_id = id.value();
    }
    else
    {
        std::cerr << "Error: Group name not found" << std::endl;
        m_id = std::numeric_limits<uint32_t>::max();
    }
}

bool PosixGroup::operator==(const PosixGroup& other) const noexcept
{
    return (m_id == other.m_id);
}

PosixGroup PosixGroup::getGroupOfCurrentProcess() noexcept
{
    return PosixGroup(getegid());
}

cxx::optional<gid_t> PosixGroup::getGroupID(const PosixGroup::string_t& f_name) noexcept
{
    auto getgrnamCall = posixCall(getgrnam)(f_name.c_str()).failureReturnValue(nullptr).evaluate();

    if (getgrnamCall.has_error())
    {
        std::cerr << "Error: Could not find group '" << f_name << "'." << std::endl;
        return cxx::nullopt_t();
    }

    return cxx::make_optional<gid_t>(getgrnamCall->value->gr_gid);
}

cxx::optional<PosixGroup::string_t> PosixGroup::getGroupName(gid_t f_id) noexcept
{
    auto getgrgidCall = posixCall(getgrgid)(f_id).failureReturnValue(nullptr).evaluate();

    if (getgrgidCall.has_error())
    {
        std::cerr << "Error: Could not find group with id '" << f_id << "'." << std::endl;
        return cxx::nullopt_t();
    }

    return cxx::make_optional<string_t>(string_t(iox::cxx::TruncateToCapacity, getgrgidCall->value->gr_name));
}

PosixGroup::string_t PosixGroup::getName() const noexcept
{
    auto name = getGroupName(m_id);
    if (name.has_value())
    {
        return name.value();
    }
    else
    {
        return string_t();
    }
}

gid_t PosixGroup::getID() const noexcept
{
    return m_id;
}

bool PosixGroup::doesExist() const noexcept
{
    return m_doesExist;
}

cxx::optional<uid_t> PosixUser::getUserID(const PosixGroup::string_t& f_name) noexcept
{
    auto getpwnamCall = posixCall(getpwnam)(f_name.c_str()).failureReturnValue(nullptr).evaluate();

    if (getpwnamCall.has_error())
    {
        std::cerr << "Error: Could not find user '" << f_name << "'." << std::endl;
        return cxx::nullopt_t();
    }
    return cxx::make_optional<uid_t>(getpwnamCall->value->pw_uid);
}

cxx::optional<PosixUser::string_t> PosixUser::getUserName(uid_t f_id) noexcept
{
    auto getpwuidCall = posixCall(getpwuid)(f_id).failureReturnValue(nullptr).evaluate();

    if (getpwuidCall.has_error())
    {
        std::cerr << "Error: Could not find user with id'" << f_id << "'." << std::endl;
        return cxx::nullopt_t();
    }
    return cxx::make_optional<string_t>(string_t(iox::cxx::TruncateToCapacity, getpwuidCall->value->pw_name));
}

PosixUser::groupVector_t PosixUser::getGroups() const noexcept
{
    auto userName = getUserName(m_id);
    if (!userName.has_value())
    {
        return groupVector_t();
    }

    auto getpwnamCall = posixCall(getpwnam)(userName->c_str()).failureReturnValue(nullptr).evaluate();
    if (getpwnamCall.has_error())
    {
        std::cerr << "Error: getpwnam call failed" << std::endl;
        return groupVector_t();
    }

    gid_t userDefaultGroup = getpwnamCall->value->pw_gid;

    gid_t groups[MaxNumberOfGroups];
    int32_t numGroups = MaxNumberOfGroups;

    auto getgrouplistCall = posixCall(iox_getgrouplist)(userName->c_str(), userDefaultGroup, groups, &numGroups)
                                .failureReturnValue(-1)
                                .evaluate();
    if (getgrouplistCall.has_error())
    {
        std::cerr << "Error: Could not obtain group list" << std::endl;
        return groupVector_t();
    }

    if (numGroups == -1)
    {
        std::cerr << "Error: List with negative size returned" << std::endl;
        return groupVector_t();
    }

    groupVector_t vec;
    for (int32_t i = 0; i < numGroups; ++i)
    {
        vec.emplace_back(PosixGroup(groups[i]));
    }

    return vec;
}

PosixUser::PosixUser(uid_t f_id) noexcept
    : m_id(f_id)
    , m_doesExist(getUserName(f_id).has_value())
{
}

PosixUser::PosixUser(const PosixUser::string_t& f_name) noexcept
{
    auto id = getUserID(f_name);
    if (id.has_value())
    {
        m_id = id.value();
    }
    else
    {
        std::cerr << "Error: User name not found" << std::endl;
        m_id = std::numeric_limits<uint32_t>::max();
    }
}

PosixUser::string_t PosixUser::getName() const noexcept
{
    auto name = getUserName(m_id);
    if (name.has_value())
    {
        return name.value();
    }
    else
    {
        return string_t();
    }
}

uid_t PosixUser::getID() const noexcept
{
    return m_id;
}

bool PosixUser::doesExist() const noexcept
{
    return m_doesExist;
}

PosixUser PosixUser::getUserOfCurrentProcess() noexcept
{
    return PosixUser(geteuid());
}

} // namespace posix
} // namespace iox
