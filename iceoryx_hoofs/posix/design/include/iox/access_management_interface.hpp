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
#ifndef IOX_HOOFS_POSIX_DESIGN_ACCESS_MANAGEMENT_INTERFACE_HPP
#define IOX_HOOFS_POSIX_DESIGN_ACCESS_MANAGEMENT_INTERFACE_HPP

#include "iceoryx_platform/types.hpp"
#include "iox/expected.hpp"
#include "iox/filesystem.hpp"
#include "iox/group_name.hpp"
#include "iox/optional.hpp"
#include "iox/user_name.hpp"

#include <limits>

namespace iox
{
enum class FileStatError
{
};

enum class FileSetOwnerError
{
};

enum class FileSetPermissionError
{
};

class Ownership
{
  public:
    uid_t uid() const noexcept;
    gid_t gid() const noexcept;

    static optional<Ownership> from_user_and_group(const uid_t uid, const gid_t gid) noexcept;
    static optional<Ownership> from_user_and_group(const UserName& user_name, const GroupName& group_name) noexcept;

  private:
    Ownership(const uid_t uid, const gid_t gid) noexcept;

  private:
    uid_t m_uid{std::numeric_limits<pid_t>::max()};
    gid_t m_gid{std::numeric_limits<gid_t>::max()};
};

template <typename Derived>
struct AccessManagementInterface
{
    expected<Ownership, FileStatError> get_ownership() const noexcept;
    expected<FileSetOwnerError> set_ownership(const Ownership& owner) noexcept;
    expected<access_rights, FileStatError> get_permissions() const noexcept;
    expected<FileSetPermissionError> set_permissions(const access_rights permissions) noexcept;
};
} // namespace iox

#include "detail/access_management_interface.inl"

#endif
