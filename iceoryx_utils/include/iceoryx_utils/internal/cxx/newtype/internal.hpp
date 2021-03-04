// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_CXX_NEWTYPE_INTERNAL_HPP
#define IOX_UTILS_CXX_NEWTYPE_INTERNAL_HPP

#include <utility>

namespace iox
{
namespace cxx
{
template <typename, template <typename> class...>
class NewType;
namespace newtype
{
namespace internal
{
struct ProtectedConstructor_t
{
};

static constexpr ProtectedConstructor_t ProtectedConstructor = ProtectedConstructor_t();

template <typename T>
inline typename T::value_type newTypeAccessor(const T& b) noexcept
{
    return b.m_value;
}
} // namespace internal
} // namespace newtype
} // namespace cxx
} // namespace iox

#endif
