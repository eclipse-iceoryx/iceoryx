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
class NewTypeBaseFriend
{
  protected:
    template <typename T, typename NewBaseType>
    static T getValue(const NewBaseType& b) noexcept
    {
        return b.value();
    }
};

template <typename T>
class NewTypeBase
{
  public:
    explicit NewTypeBase(const T& value) noexcept;

    NewTypeBase(const NewTypeBase&) = default;
    NewTypeBase(NewTypeBase&&) = default;

    NewTypeBase& operator=(const NewTypeBase&) = default;
    NewTypeBase& operator=(NewTypeBase&&) = default;

    friend class NewTypeBaseFriend;

  private:
    operator T() const noexcept;

    const T& value() const& noexcept;
    T& value() & noexcept;
    const T&& value() const&& noexcept;
    T&& value() && noexcept;

  private:
    T m_value;
};
} // namespace newtype
} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/newtype/newtype_base.inl"

#endif
