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
#ifndef IOX_UTILS_CXX_NEWTYPE_NEWTYPE_BASE_HPP
#define IOX_UTILS_CXX_NEWTYPE_NEWTYPE_BASE_HPP

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
template <typename T>
inline typename T::value_type newTypeBaseAccessor(const T& b) noexcept
{
    return b.m_value;
}
} // namespace internal

template <typename T>
class NewTypeBase
{
  protected:
    template <typename U = T>
    explicit NewTypeBase(const U& value) noexcept
    {
    }

    template <typename Type>
    friend typename Type::value_type internal::newTypeBaseAccessor(const Type&) noexcept;

  private:
    T m_value;
};
} // namespace newtype
} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/newtype/newtype_base.inl"

#endif
