// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/posix_user.hpp"
#include "iceoryx_platform/grp.hpp"
#include "iceoryx_platform/platform_correction.hpp"
#include "iceoryx_platform/pwd.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_platform/unistd.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"
#include "iox/uninitialized_array.hpp"

#include <limits>

namespace iox
{
optional<iox_uid_t> PosixUser::getUserID(const userName_t& name) noexcept
{
    auto getpwnamCall = IOX_POSIX_CALL(getpwnam)(name.c_str()).failureReturnValue(nullptr).evaluate();

    if (getpwnamCall.has_error())
    {
        IOX_LOG(ERROR, "Error: Could not find user '" << name << "'.");
        return nullopt_t();
    }
    return make_optional<iox_uid_t>(getpwnamCall->value->pw_uid);
}

optional<PosixUser::userName_t> PosixUser::getUserName(iox_uid_t id) noexcept
{
    auto getpwuidCall = IOX_POSIX_CALL(getpwuid)(id).failureReturnValue(nullptr).evaluate();

    if (getpwuidCall.has_error())
    {
        IOX_LOG(ERROR, "Error: Could not find user with id'" << id << "'.");
        return nullopt_t();
    }
    return make_optional<userName_t>(userName_t(iox::TruncateToCapacity, getpwuidCall->value->pw_name));
}

PosixUser::groupVector_t PosixUser::getGroups() const noexcept
{
    auto userName = getUserName(m_id);
    if (!userName.has_value())
    {
        return groupVector_t();
    }

    auto getpwnamCall = IOX_POSIX_CALL(getpwnam)(userName->c_str()).failureReturnValue(nullptr).evaluate();
    if (getpwnamCall.has_error())
    {
        IOX_LOG(ERROR, "Error: getpwnam call failed");
        return groupVector_t();
    }

    iox_gid_t userDefaultGroup = getpwnamCall->value->pw_gid;
    UninitializedArray<iox_gid_t, MAX_NUMBER_OF_GROUPS> groups{}; // groups is initialized in iox_getgrouplist
    int numGroups = MAX_NUMBER_OF_GROUPS;

    auto getgrouplistCall =
        IOX_POSIX_CALL(iox_getgrouplist)(userName->c_str(), userDefaultGroup, &groups[0], &numGroups)
            .failureReturnValue(-1)
            .evaluate();
    if (getgrouplistCall.has_error())
    {
        IOX_LOG(ERROR, "Error: Could not obtain group list");
        return groupVector_t();
    }

    if (numGroups == -1)
    {
        IOX_LOG(ERROR, "Error: List with negative size returned");
        return groupVector_t();
    }

    groupVector_t vec;
    for (int32_t i = 0; i < numGroups; ++i)
    {
        vec.emplace_back(groups[static_cast<uint64_t>(i)]);
    }

    return vec;
}

PosixUser::PosixUser(iox_uid_t id) noexcept
    : m_id(id)
    , m_doesExist(getUserName(id).has_value())
{
}

PosixUser::PosixUser(const PosixUser::userName_t& name) noexcept
{
    auto id = getUserID(name);
    if (id.has_value())
    {
        m_id = id.value();
    }
    else
    {
        IOX_LOG(ERROR, "Error: User name not found");
        m_id = std::numeric_limits<iox_gid_t>::max();
    }
}

PosixUser::userName_t PosixUser::getName() const noexcept
{
    auto name = getUserName(m_id);
    if (name.has_value())
    {
        return name.value();
    }

    return userName_t();
}

iox_uid_t PosixUser::getID() const noexcept
{
    return m_id;
}

bool PosixUser::doesExist() const noexcept
{
    return m_doesExist;
}

PosixUser PosixUser::getUserOfCurrentProcess() noexcept
{
    return PosixUser(iox_geteuid());
}

} // namespace iox
