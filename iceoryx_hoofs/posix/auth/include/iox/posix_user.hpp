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

#ifndef IOX_HOOFS_POSIX_AUTH_POSIX_USER_HPP
#define IOX_HOOFS_POSIX_AUTH_POSIX_USER_HPP

#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_platform/types.hpp"
#include "iox/optional.hpp"
#include "iox/posix_group.hpp"
#include "iox/string.hpp"
#include "iox/vector.hpp"

#include <string>

namespace iox
{
class PosixUser
{
  public:
    // Increase from 888 to 1024 to support systems with many supplementary groups
    // (e.g. Nvidia Tegra boards where `id -G | wc -w` returns 1000+).
    // When the group count exceeds MAX_NUMBER_OF_GROUPS, iox_getgrouplist fails
    // with "Could not obtain group list", causing RouDi to deny the node a
    // writable SHM segment, which crashes the node with:
    //   POSH__RUNTIME_NO_WRITABLE_SHM_SEGMENT / EXPECTS_ENSURES_FAILED
    static constexpr uint64_t MAX_NUMBER_OF_GROUPS = 1024;
    using groupVector_t = vector<PosixGroup, MAX_NUMBER_OF_GROUPS>;

    using userName_t = string<platform::MAX_USER_NAME_LENGTH>;

    explicit PosixUser(const iox_uid_t id) noexcept;
    explicit PosixUser(const userName_t& name) noexcept;

    groupVector_t getGroups() const noexcept;
    userName_t getName() const noexcept;
    iox_uid_t getID() const noexcept;

    bool doesExist() const noexcept;

    static PosixUser getUserOfCurrentProcess() noexcept;

    static optional<iox_uid_t> getUserID(const userName_t& name) noexcept;
    static optional<userName_t> getUserName(iox_uid_t id) noexcept;

  private:
    iox_uid_t m_id;
    bool m_doesExist{false};
};

} // namespace iox

#endif // IOX_HOOFS_POSIX_AUTH_POSIX_USER_HPP
