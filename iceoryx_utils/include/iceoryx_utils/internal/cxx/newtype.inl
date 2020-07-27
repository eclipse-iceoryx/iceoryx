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
#ifndef IOX_UTILS_CXX_NEWTYPE_INL
#define IOX_UTILS_CXX_NEWTYPE_INL

namespace iox
{
namespace cxx
{
template <typename T, template <typename> class... Policies>
inline NewType<T, Policies...>::NewType() noexcept
{
    static_assert(algorithm::doesContainType<newtype::DefaultConstructable<T>, Policies<T>...>(),
                  "This type is not default constructable, please add the newtype::DefaultConstructable policy.");
}

template <typename T, template <typename> class... Policies>
inline NewType<T, Policies...>::NewType(const T& rhs) noexcept
    : newtype::NewTypeBase<T>(rhs)
{
    static_assert(
        algorithm::doesContainType<newtype::ConstructByValueCopy<T>, Policies<T>...>(),
        "This type is not constructable with value copy, please add the newtype::ConstructByValueCopy policy.");
}

template <typename T, template <typename> class... Policies>
inline NewType<T, Policies...>::NewType(const NewType& rhs) noexcept
    : newtype::NewTypeBase<T>(rhs)
{
    static_assert(algorithm::doesContainType<newtype::Copyable<T>, Policies<T>...>(),
                  "This type is not copyable, please add the newtype::Copyable policy.");
}

template <typename T, template <typename> class... Policies>
inline NewType<T, Policies...>::NewType(NewType&& rhs) noexcept
    : newtype::NewTypeBase<T>(std::move(rhs))
{
    static_assert(algorithm::doesContainType<newtype::Movable<T>, Policies<T>...>(),
                  "This type is not movable, please add the newtype::Movable policy.");
}

template <typename T, template <typename> class... Policies>
inline NewType<T, Policies...>& NewType<T, Policies...>::operator=(const NewType& rhs) noexcept
{
    *reinterpret_cast<newtype::NewTypeBase<T>*>(this) = reinterpret_cast<const newtype::NewTypeBase<T>&>(rhs);
    static_assert(algorithm::doesContainType<newtype::Copyable<T>, Policies<T>...>(),
                  "This type is not copyable, please add the newtype::Copyable policy.");
    return *this;
}

template <typename T, template <typename> class... Policies>
inline NewType<T, Policies...>& NewType<T, Policies...>::operator=(NewType&& rhs) noexcept
{
    *reinterpret_cast<newtype::NewTypeBase<T>*>(this) =
        std::move(reinterpret_cast<const newtype::NewTypeBase<T>&>(rhs));
    static_assert(algorithm::doesContainType<newtype::Movable<T>, Policies<T>...>(),
                  "This type is not movable, please add the newtype::Movable policy.");
    return *this;
}

template <typename T, template <typename> class... Policies>
inline NewType<T, Policies...>::operator T() const noexcept
{
    static_assert(algorithm::doesContainType<newtype::Convertable<T>, Policies<T>...>(),
                  "This type is not convertable, please add the newtype::Convertable policy.");
    return T();
}

} // namespace cxx
} // namespace iox

#endif
