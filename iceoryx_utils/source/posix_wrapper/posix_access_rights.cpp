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

#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

#include <grp.h>
#include <limits>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace iox
{
namespace posix
{
PosixRights::PosixRights(bool f_read, bool f_write, bool f_execute)
    : m_read(f_read)
    , m_write(f_write)
    , m_execute(f_execute)
{
}

PosixGroup::PosixGroup(gid_t f_id)
    : m_id(f_id)
    , m_doesExist(getGroupName(f_id).has_value())
{
}

PosixGroup::PosixGroup(PosixGroup::string_t f_name)
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

bool PosixGroup::operator==(const PosixGroup& other) const
{
    return (m_id == other.m_id);
}

PosixGroup PosixGroup::getGroupOfCurrentProcess()
{
    return PosixGroup(getegid());
}

cxx::optional<gid_t> PosixGroup::getGroupID(const PosixGroup::string_t& f_name)
{
    auto getgrnamCall = cxx::makeSmartC(
        getgrnam, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {static_cast<struct group*>(nullptr)}, {}, f_name.c_str());

    if (getgrnamCall.hasErrors())
    {
        std::cerr << "Error: Could not find group '" << f_name << "'." << std::endl;
        return cxx::nullopt_t();
    }

    return cxx::make_optional<gid_t>(getgrnamCall.getReturnValue()->gr_gid);
}

cxx::optional<PosixGroup::string_t> PosixGroup::getGroupName(gid_t f_id)
{
    auto getgrgidCall = cxx::makeSmartC(
        getgrgid, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {static_cast<struct group*>(nullptr)}, {}, f_id);

    if (getgrgidCall.hasErrors())
    {
        std::cerr << "Error: Could not find group with id '" << f_id << "'." << std::endl;
        return cxx::nullopt_t();
    }

    return cxx::make_optional<string_t>(getgrgidCall.getReturnValue()->gr_name);
}

PosixGroup::string_t PosixGroup::getName() const
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

gid_t PosixGroup::getID() const
{
    return m_id;
}

bool PosixGroup::doesExist() const
{
    return m_doesExist;
}

cxx::optional<uid_t> PosixUser::getUserID(const PosixGroup::string_t& f_name)
{
    auto getpwnamCall = cxx::makeSmartC(
        getpwnam, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {static_cast<struct passwd*>(nullptr)}, {}, f_name.c_str());

    if (getpwnamCall.hasErrors())
    {
        std::cerr << "Error: Could not find user '" << f_name << "'." << std::endl;
        return cxx::nullopt_t();
    }
    return cxx::make_optional<uid_t>(getpwnamCall.getReturnValue()->pw_uid);
}

cxx::optional<PosixUser::string_t> PosixUser::getUserName(uid_t f_id)
{
    auto getpwnamCall = cxx::makeSmartC(
        getpwuid, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {static_cast<struct passwd*>(nullptr)}, {}, f_id);

    if (getpwnamCall.hasErrors())
    {
        std::cerr << "Error: Could not find user with id'" << f_id << "'." << std::endl;
        return cxx::nullopt_t();
    }
    return cxx::make_optional<string_t>(getpwnamCall.getReturnValue()->pw_name);
}

PosixUser::groupVector_t PosixUser::getGroups() const
{
    auto userName = getUserName(m_id);
    if (!userName.has_value())
    {
        return groupVector_t();
    }

    auto getpwnamCall = cxx::makeSmartC(getpwnam,
                                        cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                        {static_cast<struct passwd*>(nullptr)},
                                        {},
                                        userName->c_str());

    if (getpwnamCall.hasErrors())
    {
        std::cerr << "Error: getpwnam call failed" << std::endl;
        return groupVector_t();
    }

    gid_t userDefaultGroup = getpwnamCall.getReturnValue()->pw_gid;

    gid_t groups[MaxNumberOfGroups];
    int numGroups = MaxNumberOfGroups;

    auto getgrouplistCall = cxx::makeSmartC(getgrouplist,
                                            cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                            {-1},
                                            {},
                                            userName->c_str(),
                                            userDefaultGroup,
                                            groups,
                                            &numGroups);

    if (getgrouplistCall.hasErrors())
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
    for (int i = 0; i < numGroups; ++i)
    {
        vec.emplace_back(PosixGroup(groups[i]));
    }

    return vec;
}

PosixUser::PosixUser(uid_t f_id)
    : m_id(f_id)
    , m_doesExist(getUserName(f_id).has_value())
{
}

PosixUser::PosixUser(const PosixUser::string_t& f_name)
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

PosixUser::string_t PosixUser::getName() const
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

uid_t PosixUser::getID() const
{
    return m_id;
}

bool PosixUser::doesExist() const
{
    return m_doesExist;
}

PosixUser PosixUser::getUserOfCurrentProcess()
{
    return PosixUser(geteuid());
}

} // namespace posix
} // namespace iox
