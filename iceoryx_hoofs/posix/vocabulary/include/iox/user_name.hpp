// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_VOCABULARY_USER_NAME_HPP
#define IOX_HOOFS_POSIX_VOCABULARY_USER_NAME_HPP

#include "iox/semantic_string.hpp"

namespace iox
{
namespace detail
{
bool user_name_does_contain_invalid_characters(const string<platform::MAX_USER_NAME_LENGTH>& value) noexcept;
bool user_name_does_contain_invalid_content(const string<platform::MAX_USER_NAME_LENGTH>& value) noexcept;
} // namespace detail

/// @brief Represents a POSIX user name
class UserName : public SemanticString<UserName,
                                       platform::MAX_USER_NAME_LENGTH,
                                       detail::user_name_does_contain_invalid_content,
                                       detail::user_name_does_contain_invalid_characters>
{
    using Parent = SemanticString<UserName,
                                  platform::MAX_USER_NAME_LENGTH,
                                  detail::user_name_does_contain_invalid_content,
                                  detail::user_name_does_contain_invalid_characters>;
    using Parent::Parent;
};
} // namespace iox

#endif
