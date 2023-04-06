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

#include "iox/file_management_interface.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace details
{
expected<iox_stat, FileStatError> get_file_status(const int fildes) noexcept
{
    iox_stat file_status = {};
    auto result = posix::posixCall(iox_fstat)(fildes, &file_status).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EIO:
            IOX_LOG(ERROR) << "Unable to acquire file status since an io failure occurred while reading.";
            return iox::error<FileStatError>(FileStatError::IoFailure);
        case EOVERFLOW:
            IOX_LOG(ERROR) << "Unable to acquire file status since the file size cannot be represented by the "
                              "corresponding structure.";
            return iox::error<FileStatError>(FileStatError::FileTooLarge);
        default:
            IOX_LOG(ERROR) << "Unable to acquire file status due to an unknown failure";
            return iox::error<FileStatError>(FileStatError::UnknownError);
        }
    }

    return iox::success<iox_stat>(file_status);
}

expected<FileSetOwnerError> set_owner(const int fildes, const uid_t uid, const gid_t gid) noexcept
{
    auto result = posix::posixCall(iox_fchown)(fildes, uid, gid).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EPERM:
            IOX_LOG(ERROR) << "Unable to set owner due to insufficient permissions.";
            return iox::error<FileSetOwnerError>(FileSetOwnerError::PermissionDenied);
        case EROFS:
            IOX_LOG(ERROR) << "Unable to set owner since it is a read-only filesystem.";
            return iox::error<FileSetOwnerError>(FileSetOwnerError::ReadOnlyFilesystem);
        case EINVAL:
            IOX_LOG(ERROR) << "Unable to set owner since the uid " << uid << " or the gid " << gid
                           << " are not supported by the OS implementation.";
            return iox::error<FileSetOwnerError>(FileSetOwnerError::InvalidUidOrGid);
        case EIO:
            IOX_LOG(ERROR) << "Unable to set owner due to an IO error.";
            return iox::error<FileSetOwnerError>(FileSetOwnerError::IoFailure);
        case EINTR:
            IOX_LOG(ERROR) << "Unable to set owner since an interrupt was received.";
            return iox::error<FileSetOwnerError>(FileSetOwnerError::Interrupt);
        default:
            IOX_LOG(ERROR) << "Unable to set owner since an unknown error occurred.";
            return iox::error<FileSetOwnerError>(FileSetOwnerError::UnknownError);
        }
    }

    return iox::success<>();
}

expected<FileSetPermissionError> set_permissions(const int fildes, const access_rights perms) noexcept
{
    auto result = posix::posixCall(iox_fchmod)(fildes, perms.value()).failureReturnValue(-1).evaluate();

    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case EPERM:
            IOX_LOG(ERROR) << "Unable to adjust permissions due to insufficient permissions.";
            return iox::error<FileSetPermissionError>(FileSetPermissionError::PermissionDenied);
        case EROFS:
            IOX_LOG(ERROR) << "Unable to adjust permissions since it is a read-only filesystem.";
            return iox::error<FileSetPermissionError>(FileSetPermissionError::ReadOnlyFilesystem);
        default:
            IOX_LOG(ERROR) << "Unable to adjust permissions since an unknown error occurred.";
            return iox::error<FileSetPermissionError>(FileSetPermissionError::UnknownError);
        }
    }

    return iox::success<>();
}

} // namespace details

uid_t Ownership::uid() const noexcept
{
    return m_uid;
}

gid_t Ownership::gid() const noexcept
{
    return m_gid;
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
