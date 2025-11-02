// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#ifndef IOX_HOOFS_POSIX_AUTH_POSIX_GROUP_HPP
#define IOX_HOOFS_POSIX_AUTH_POSIX_GROUP_HPP

#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_platform/types.hpp"
#include "iox/optional.hpp"
#include "iox/string.hpp"

#include <string>

namespace iox
{
class PosixGroup
{
  public:
    using groupName_t = string<platform::MAX_GROUP_NAME_LENGTH>;
    explicit PosixGroup(const iox_gid_t id) noexcept;
    explicit PosixGroup(const groupName_t& name) noexcept;

    bool operator==(const PosixGroup& other) const noexcept;

    groupName_t getName() const noexcept;
    iox_gid_t getID() const noexcept;

    bool doesExist() const noexcept;

    static PosixGroup getGroupOfCurrentProcess() noexcept;

    static optional<iox_uid_t> getGroupID(const groupName_t& name) noexcept;
    static optional<groupName_t> getGroupName(iox_gid_t id) noexcept;

  private:
    iox_gid_t m_id;
    bool m_doesExist{false};
};

} // namespace iox

#endif // IOX_HOOFS_POSIX_AUTH_POSIX_GROUP_HPP
