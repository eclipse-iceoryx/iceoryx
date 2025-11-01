// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_DESIGN_NEWTYPE_PROTECTED_CONSTRUCTOR_HPP
#define IOX_HOOFS_DESIGN_NEWTYPE_PROTECTED_CONSTRUCTOR_HPP

namespace iox
{
namespace newtype
{
template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a default'ed destructor does not define a
// destructor, hence the copy/move operations are not deleted. The only adaptation is that the dtor is protected to
// prohibit the user deleting the child type by explicitly calling the destructor of the base type. Additionally, this
// is a marker struct that adds only the described property to the new type. Adding copy/move operations would
// contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct ProtectedConstructByValueCopy
{
  protected:
    ~ProtectedConstructByValueCopy() = default;
};
} // namespace newtype
} // namespace iox

#endif
