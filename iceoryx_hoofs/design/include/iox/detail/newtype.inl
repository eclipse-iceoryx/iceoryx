// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_DESIGN_NEWTYPE_INL
#define IOX_HOOFS_DESIGN_NEWTYPE_INL

#include "iox/newtype.hpp"

namespace iox
{
template <typename Derived, typename T, template <typename, typename> class... Policies>
// AXIVION Next Construct AutosarC++19_03-A12.6.1 : m_value is initialized by the default constructor of T; the code
// will not compile, if the default constructor of T does not exist or the DefaultConstructable policy is not added
inline NewType<Derived, T, Policies...>::NewType() noexcept
{
    static_assert(algorithm::doesContainType<newtype::DefaultConstructable<Derived, T>, Policies<Derived, T>...>(),
                  "This type is not default constructable, please add the newtype::DefaultConstructable policy.");
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : delegating wanted only to "ProtectedConstructByValueCopy"
// constructor of T
inline NewType<Derived, T, Policies...>::NewType(newtype::internal::ProtectedConstructor_t, const T& rhs) noexcept
    : m_value(rhs)
{
    static_assert(
        algorithm::doesContainType<newtype::ProtectedConstructByValueCopy<Derived, T>, Policies<Derived, T>...>(),
        "This type is not protected constructable with value copy, please add the "
        "newtype::ProtectedConstructByValueCopy policy.");
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::NewType(const T& rhs) noexcept
    : m_value(rhs)
{
    static_assert(
        algorithm::doesContainType<newtype::ConstructByValueCopy<Derived, T>, Policies<Derived, T>...>(),
        "This type is not constructable with value copy, please add the newtype::ConstructByValueCopy policy.");
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::NewType(const NewType& rhs) noexcept
    : m_value(rhs.m_value)
{
    static_assert(algorithm::doesContainType<newtype::CopyConstructable<Derived, T>, Policies<Derived, T>...>(),
                  "This type is not copy constructable, please add the newtype::CopyConstructable policy.");
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::NewType(NewType&& rhs) noexcept
    : m_value(std::move(rhs.m_value))
{
    static_assert(algorithm::doesContainType<newtype::MoveConstructable<Derived, T>, Policies<Derived, T>...>(),
                  "This type is not move constructable, please add the newtype::MoveConstructable policy.");
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>& NewType<Derived, T, Policies...>::operator=(const NewType& rhs) noexcept
{
    if (this != &rhs)
    {
        m_value = rhs.m_value;
        static_assert(algorithm::doesContainType<newtype::CopyAssignable<Derived, T>, Policies<Derived, T>...>(),
                      "This type is not copy assignable, please add the newtype::CopyAssignable policy.");
    }
    return *this;
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>& NewType<Derived, T, Policies...>::operator=(NewType&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_value = std::move(rhs.m_value);
    }
    static_assert(algorithm::doesContainType<newtype::MoveAssignable<Derived, T>, Policies<Derived, T>...>(),
                  "This type is not move assignable, please add the newtype::MoveAssignable policy.");
    return *this;
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>& NewType<Derived, T, Policies...>::operator=(const T& rhs) noexcept
{
    m_value = rhs;
    static_assert(algorithm::doesContainType<newtype::AssignByValueCopy<Derived, T>, Policies<Derived, T>...>(),
                  "This type is not assignable by value copy, please add the newtype::AssignByValueCopy policy.");
    return *this;
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>& NewType<Derived, T, Policies...>::operator=(T&& rhs) noexcept
{
    m_value = std::move(rhs);
    static_assert(algorithm::doesContainType<newtype::AssignByValueMove<Derived, T>, Policies<Derived, T>...>(),
                  "This type is not assignable by value move, please add the newtype::AssignByValueMove policy.");
    return *this;
}

template <typename Derived, typename T, template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::operator T() const noexcept
{
    static_assert(algorithm::doesContainType<newtype::Convertable<Derived, T>, Policies<Derived, T>...>(),
                  "This type is not convertable, please add the newtype::Convertable policy.");
    return m_value;
}

} // namespace iox

#endif
