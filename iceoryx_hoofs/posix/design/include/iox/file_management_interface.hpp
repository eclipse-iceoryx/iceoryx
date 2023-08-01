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
#ifndef IOX_HOOFS_POSIX_DESIGN_FILE_MANAGEMENT_INTERFACE_HPP
#define IOX_HOOFS_POSIX_DESIGN_FILE_MANAGEMENT_INTERFACE_HPP

#include "iceoryx_platform/stat.hpp"
#include "iceoryx_platform/types.hpp"
#include "iox/expected.hpp"
#include "iox/filesystem.hpp"
#include "iox/group_name.hpp"
#include "iox/optional.hpp"
#include "iox/user_name.hpp"

#include <limits>

namespace iox
{
/// @brief Describes failures when acquiring details about a file.
enum class FileStatError
{
    IoFailure,
    FileTooLarge,
    UnknownError,
};

/// @brief Describes failures when setting the owner of a file
enum class FileSetOwnerError
{
    IoFailure,
    Interrupt,
    PermissionDenied,
    ReadOnlyFilesystem,
    InvalidUidOrGid,
    UnknownError,
};

/// @brief Describes failures when setting the permissions of a file
enum class FileSetPermissionError
{
    PermissionDenied,
    ReadOnlyFilesystem,
    UnknownError,
};

namespace details
{
expected<iox_stat, FileStatError> get_file_status(const int fildes) noexcept;
expected<void, FileSetOwnerError> set_owner(const int fildes, const uid_t uid, const gid_t gid) noexcept;
expected<void, FileSetPermissionError> set_permissions(const int fildes, const access_rights perms) noexcept;
} // namespace details

/// @brief Represents the POSIX owners (user and group) of a file.
class Ownership
{
  public:
    /// @brief Acquire the user id
    /// @returns uid
    uid_t uid() const noexcept;

    /// @brief Acquire the group id
    /// @returns gid
    gid_t gid() const noexcept;

    /// @brief Constructs a ownership object from a uid and a gid.
    /// @returns If the user or group does not exist it returns 'cxx::nullopt' otherwise an Ownership object
    ///             with existing user and group
    static optional<Ownership> from_user_and_group(const uid_t uid, const gid_t gid) noexcept;

    /// @brief Constructs a ownership object from a user name and a group name.
    /// @returns If the user or group does not exist it returns 'cxx::nullopt' otherwise an Ownership object
    ///             with existing user and group
    static optional<Ownership> from_user_and_group(const UserName& user_name, const GroupName& group_name) noexcept;

    /// @brief Returns the user and group owner of the current process.
    static Ownership from_process() noexcept;

  private:
    template <typename>
    friend struct FileManagementInterface;
    Ownership(const uid_t uid, const gid_t gid) noexcept;

  private:
    uid_t m_uid{std::numeric_limits<uid_t>::max()};
    gid_t m_gid{std::numeric_limits<gid_t>::max()};
};

/// @brief Abstract implementation to manage things common to all file descriptor
///        based constructs like ownership and permissions.
/// @note Can be used by every class which provides the method 'get_file_handle'
///       via inheritance.
/// @code
///   class MyResourceBasedOnFileDescriptor: public FileManagementInterface<MyResourceBasedOnFileDescriptor> {
///     public:
///       // must be implemented
///       int get_file_handle() const noexcept;
///   };
/// @endcode
template <typename Derived>
struct FileManagementInterface
{
    /// @brief Returns the owners of the underlying file descriptor.
    /// @return On failure a 'FileStatError' describing the error otherwise 'Ownership'.
    expected<Ownership, FileStatError> get_ownership() const noexcept;

    /// @brief Sets the owners of the underlying file descriptor.
    /// @param[in] owner the new owners of the file descriptor
    /// @return On failure a 'FileSetOwnerError' describing the error.
    expected<void, FileSetOwnerError> set_ownership(const Ownership owner) noexcept;

    /// @brief Returns the permissions of the underlying file descriptor.
    /// @return On failure a 'FileStatError' describing the error otherwise 'access_rights'.
    expected<access_rights, FileStatError> get_permissions() const noexcept;

    /// @brief Sets the permissions of the underlying file descriptor.
    /// @param[in] permissions the new permissions of the file descriptor
    /// @return On failure a 'FileSetPermissionError' describing the error.
    expected<void, FileSetPermissionError> set_permissions(const access_rights permissions) noexcept;

    /// @brief Returns the size of the corresponding file.
    /// @return On failure a 'FileStatError' describing the error otherwise the size.
    expected<uint64_t, FileStatError> get_size() const noexcept;
};
} // namespace iox

#include "detail/file_management_interface.inl"

#endif
