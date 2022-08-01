// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_NEWTYPE_INCREMENTABLE_HPP
#define IOX_HOOFS_CXX_NEWTYPE_INCREMENTABLE_HPP
#include "iox/detail/newtype/internal.hpp"

namespace iox
{
namespace newtype
{
template <typename T>
// not required since a default'ed destructor does not define a destructor, hence the copy/move operations are
// not deleted.
// the only adaptation is that the dtor is protected to prohibit the user deleting the child type by
// explicitly calling the destructor of the base type.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
struct Incrementable
{
  protected:
    ~Incrementable() = default;
};

template <typename T>
auto operator++(T& rhs) noexcept -> typename T::value_type
{
    return internal::preIncrement(rhs);
}

template <typename T>
auto operator++(T& rhs, int) noexcept -> typename T::value_type
{
    auto value = internal::newTypeAccessor(rhs);
    internal::preIncrement(rhs);
    return value;
}

} // namespace newtype
} // namespace iox


#endif // IOX_HOOFS_CXX_NEWTYPE_INCREMENTABLE_HPP
