// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/access_management_interface.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"

namespace iox
{
uid_t Ownership::uid() const noexcept
{
    return m_uid;
}

gid_t Ownership::gid() const noexcept
{
    return m_uid;
}

optional<Ownership> Ownership::from_user_and_group(const uid_t uid, const gid_t gid) noexcept
{
    if (!posix::PosixUser(uid).doesExist() || !posix::PosixGroup(gid).doesExist())
    {
        return iox::nullopt;
    }

    return Ownership(uid, gid);
}

optional<Ownership> Ownership::from_user_and_group(const UserName& user_name, const GroupName& group_name) noexcept
{
    posix::PosixUser user(user_name.as_string());
    posix::PosixGroup group(group_name.as_string());

    if (!user.doesExist() || !group.doesExist())
    {
        return iox::nullopt;
    }

    return Ownership(user.getID(), group.getID());
}

Ownership::Ownership(const uid_t uid, const gid_t gid) noexcept
    : m_uid{uid}
    , m_gid{gid}
{
}
} // namespace iox
