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
#ifndef IOX_HOOFS_POSIX_DESIGN_FILE_MANAGEMENT_INTERFACE_INL
#define IOX_HOOFS_POSIX_DESIGN_FILE_MANAGEMENT_INTERFACE_INL

#include "iox/file_management_interface.hpp"

namespace iox
{
template <typename Derived>
inline expected<Ownership, FileStatError> FileManagementInterface<Derived>::get_ownership() const noexcept
{
    const auto& derived_this = *static_cast<const Derived*>(this);
    auto result = details::get_file_status(derived_this.get_file_handle());
    if (result.has_error())
    {
        return iox::error<FileStatError>(result.get_error());
    }

    return iox::success<Ownership>(Ownership(result->st_uid, result->st_gid));
}

template <typename Derived>
inline expected<access_rights, FileStatError> FileManagementInterface<Derived>::get_permissions() const noexcept
{
    const auto& derived_this = *static_cast<const Derived*>(this);
    auto result = details::get_file_status(derived_this.get_file_handle());
    if (result.has_error())
    {
        return iox::error<FileStatError>(result.get_error());
    }

    // st_mode also contains the file type, since we only would like to acquire the permissions
    // we have to remove the file type
    constexpr uint16_t ONLY_FILE_PERMISSIONS = 0777;
    return iox::success<access_rights>(
        access_rights(static_cast<access_rights::value_type>(result->st_mode & ONLY_FILE_PERMISSIONS)));
}

template <typename Derived>
inline expected<FileSetOwnerError> FileManagementInterface<Derived>::set_ownership(const Ownership ownership) noexcept
{
    const auto& derived_this = *static_cast<const Derived*>(this);
    auto result = details::set_owner(derived_this.get_file_handle(), ownership.uid(), ownership.gid());
    if (result.has_error())
    {
        return iox::error<FileSetOwnerError>(result.get_error());
    }

    return iox::success<>();
}

template <typename Derived>
inline expected<FileSetPermissionError>
FileManagementInterface<Derived>::set_permissions(const access_rights permissions) noexcept
{
    const auto& derived_this = *static_cast<const Derived*>(this);
    auto result = details::set_permissions(derived_this.get_file_handle(), permissions);
    if (result.has_error())
    {
        return iox::error<FileSetPermissionError>(result.get_error());
    }

    return iox::success<>();
}

template <typename Derived>
inline expected<uint64_t, FileStatError> FileManagementInterface<Derived>::get_size() const noexcept
{
    const auto& derived_this = *static_cast<const Derived*>(this);
    auto result = details::get_file_status(derived_this.get_file_handle());
    if (result.has_error())
    {
        return iox::error<FileStatError>(result.get_error());
    }

    return iox::success<uint64_t>(static_cast<uint64_t>(result->st_size));
}
} // namespace iox

#endif
