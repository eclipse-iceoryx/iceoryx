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

#ifndef IOX_HOOFS_CXX_FUNCTIONAL_INTERFACE_INL
#define IOX_HOOFS_CXX_FUNCTIONAL_INTERFACE_INL

namespace iox
{
namespace cxx
{
namespace internal
{
///////////////
// BEGIN expect
///////////////
template <typename Derived>
inline void Expect<Derived>::expect(const char* const msg) const noexcept
{
    if (!(*static_cast<const Derived*>(this)))
    {
        std::cout << msg << std::endl;
        Ensures(false);
    }
}

template <typename Derived, typename ValueType>
inline ValueType& ExpectWithValue<Derived, ValueType>::expect(const char* const msg) & noexcept
{
    Derived* derivedThis = static_cast<Derived*>(this);

    if (!(*derivedThis))
    {
        std::cout << msg << std::endl;
        Ensures(false);
    }

    return derivedThis->value();
}

template <typename Derived, typename ValueType>
inline const ValueType& ExpectWithValue<Derived, ValueType>::expect(const char* const msg) const& noexcept
{
    using Self = ExpectWithValue<Derived, ValueType>;
    return const_cast<const ValueType&>(const_cast<Self*>(this)->expect(msg));
}

template <typename Derived, typename ValueType>
inline ValueType&& ExpectWithValue<Derived, ValueType>::expect(const char* const msg) && noexcept
{
    return std::move(this->expect(msg));
}

template <typename Derived, typename ValueType>
inline const ValueType&& ExpectWithValue<Derived, ValueType>::expect(const char* const msg) const&& noexcept
{
    using Self = ExpectWithValue<Derived, ValueType>;
    return const_cast<const ValueType&&>(std::move(const_cast<Self*>(this)->expect(msg)));
}
// END expect

/////////////////
// BEGIN value_or
/////////////////
template <typename Derived, typename ValueType>
template <typename U>
inline ValueType ValueOr<Derived, ValueType>::value_or(U&& alternative) const& noexcept
{
    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (!(*derivedThis))
    {
        return std::forward<U>(alternative);
    }

    return derivedThis->value();
}

template <typename Derived, typename ValueType>
template <typename U>
inline ValueType ValueOr<Derived, ValueType>::value_or(U&& alternative) && noexcept
{
    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (!(*derivedThis))
    {
        return std::forward<U>(alternative);
    }

    return std::move(derivedThis->value());
}
// END value_or

/////////////////
// BEGIN and_then
/////////////////
template <typename Derived, typename ValueType>
inline Derived& AndThenWithValue<Derived, ValueType>::and_then(const and_then_callback_t& callable) & noexcept
{
    Derived* derivedThis = static_cast<Derived*>(this);

    if (*derivedThis)
    {
        callable(derivedThis->value());
    }

    return *derivedThis;
}

template <typename Derived, typename ValueType>
inline Derived&& AndThenWithValue<Derived, ValueType>::and_then(const and_then_callback_t& callable) && noexcept
{
    return std::move(this->and_then(callable));
}

template <typename Derived, typename ValueType>
inline const Derived&
AndThenWithValue<Derived, ValueType>::and_then(const const_and_then_callback_t& callable) const& noexcept
{
    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (*derivedThis)
    {
        callable(derivedThis->value());
    }

    return *derivedThis;
}

template <typename Derived, typename ValueType>
inline const Derived&&
AndThenWithValue<Derived, ValueType>::and_then(const const_and_then_callback_t& callable) const&& noexcept
{
    return std::move(this->and_then(callable));
}

template <typename Derived>
inline Derived& AndThen<Derived>::and_then(const and_then_callback_t& callable) & noexcept
{
    Derived* derivedThis = static_cast<Derived*>(this);

    if (*derivedThis)
    {
        callable();
    }

    return *derivedThis;
}

template <typename Derived>
inline const Derived& AndThen<Derived>::and_then(const and_then_callback_t& callable) const& noexcept
{
    using Self = AndThen<Derived>;
    return const_cast<const Derived&>(const_cast<Self*>(this)->and_then(callable));
}

template <typename Derived>
inline Derived&& AndThen<Derived>::and_then(const and_then_callback_t& callable) && noexcept
{
    return std::move(this->and_then(callable));
}

template <typename Derived>
inline const Derived&& AndThen<Derived>::and_then(const and_then_callback_t& callable) const&& noexcept
{
    using Self = AndThen<Derived>;
    return std::move(const_cast<const Derived&>(const_cast<Self*>(this)->and_then(callable)));
}
// END and_then

////////////////
// BEGIN or_else
////////////////
template <typename Derived, typename ErrorType>
inline Derived& OrElseWithValue<Derived, ErrorType>::or_else(const or_else_callback_t& callable) & noexcept
{
    Derived* derivedThis = static_cast<Derived*>(this);

    if (!(*derivedThis))
    {
        callable(derivedThis->get_error());
    }

    return *derivedThis;
}

template <typename Derived, typename ErrorType>
inline Derived&& OrElseWithValue<Derived, ErrorType>::or_else(const or_else_callback_t& callable) && noexcept
{
    return std::move(this->or_else(callable));
}

template <typename Derived, typename ErrorType>
inline const Derived&
OrElseWithValue<Derived, ErrorType>::or_else(const const_or_else_callback_t& callable) const& noexcept
{
    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (!(*derivedThis))
    {
        callable(derivedThis->get_error());
    }

    return *derivedThis;
}

template <typename Derived, typename ErrorType>
inline const Derived&&
OrElseWithValue<Derived, ErrorType>::or_else(const const_or_else_callback_t& callable) const&& noexcept
{
    return std::move(this->or_else(callable));
}

template <typename Derived>
inline Derived& OrElse<Derived>::or_else(const or_else_callback_t& callable) & noexcept
{
    Derived* derivedThis = static_cast<Derived*>(this);

    if (!(*derivedThis))
    {
        callable();
    }

    return *derivedThis;
}

template <typename Derived>
inline Derived&& OrElse<Derived>::or_else(const or_else_callback_t& callable) && noexcept
{
    return std::move(this->or_else(callable));
}

template <typename Derived>
inline const Derived& OrElse<Derived>::or_else(const or_else_callback_t& callable) const& noexcept
{
    using Self = OrElse<Derived>;
    return const_cast<const Derived&>(const_cast<Self*>(this)->or_else(callable));
}

template <typename Derived>
inline const Derived&& OrElse<Derived>::or_else(const or_else_callback_t& callable) const&& noexcept
{
    return std::move(this->or_else(callable));
}
// END or_else


} // namespace internal
} // namespace cxx
} // namespace iox
#endif
