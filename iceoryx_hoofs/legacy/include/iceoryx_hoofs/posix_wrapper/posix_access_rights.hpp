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

#ifndef IOX_HOOFS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP
#define IOX_HOOFS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/posix_group.hpp"
#include "iox/posix_user.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/posix_group.hpp' and/or 'iox/posix_user.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") posix
{
using PosixGroup IOX_DEPRECATED_SINCE(3,
                                      "Please use 'iox::PosixGroup' from 'iox/posix_group.hpp' instead.") = PosixGroup;
using PosixUser IOX_DEPRECATED_SINCE(3, "Please use 'iox::PosixUser' from 'iox/posix_user.hpp' instead.") = PosixUser;
} // namespace posix
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_POSIX_WRAPPER_POSIX_ACCESS_RIGHTS_HPP
