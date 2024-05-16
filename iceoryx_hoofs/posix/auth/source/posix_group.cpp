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

#include "iox/posix_group.hpp"
#include "iceoryx_platform/grp.hpp"
#include "iceoryx_platform/platform_correction.hpp"
#include "iceoryx_platform/types.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"
#include "iox/uninitialized_array.hpp"

#include <limits>

namespace iox
{
PosixGroup::PosixGroup(iox_gid_t id) noexcept
    : m_id(id)
    , m_doesExist(getGroupName(id).has_value())
{
}

PosixGroup::PosixGroup(const PosixGroup::groupName_t& name) noexcept
{
    auto id = getGroupID(name);
    if (id.has_value())
    {
        m_id = id.value();
    }
    else
    {
        IOX_LOG(ERROR, "Error: Group name not found");
        m_id = std::numeric_limits<iox_gid_t>::max();
    }
}

bool PosixGroup::operator==(const PosixGroup& other) const noexcept
{
    return (m_id == other.m_id);
}

PosixGroup PosixGroup::getGroupOfCurrentProcess() noexcept
{
    return PosixGroup(iox_getgid());
}

optional<iox_gid_t> PosixGroup::getGroupID(const PosixGroup::groupName_t& name) noexcept
{
    auto getgrnamCall = IOX_POSIX_CALL(getgrnam)(name.c_str()).failureReturnValue(nullptr).evaluate();

    if (getgrnamCall.has_error())
    {
        IOX_LOG(ERROR, "Error: Could not find group '" << name << "'.");
        return nullopt_t();
    }

    return make_optional<iox_gid_t>(getgrnamCall->value->gr_gid);
}

optional<PosixGroup::groupName_t> PosixGroup::getGroupName(iox_gid_t id) noexcept
{
    auto getgrgidCall = IOX_POSIX_CALL(getgrgid)(id).failureReturnValue(nullptr).evaluate();

    if (getgrgidCall.has_error())
    {
        IOX_LOG(ERROR, "Error: Could not find group with id '" << id << "'.");
        return nullopt_t();
    }

    return make_optional<groupName_t>(groupName_t(iox::TruncateToCapacity, getgrgidCall->value->gr_name));
}

PosixGroup::groupName_t PosixGroup::getName() const noexcept
{
    auto name = getGroupName(m_id);
    if (name.has_value())
    {
        return name.value();
    }

    return groupName_t();
}

iox_gid_t PosixGroup::getID() const noexcept
{
    return m_id;
}

bool PosixGroup::doesExist() const noexcept
{
    return m_doesExist;
}
} // namespace iox
