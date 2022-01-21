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
template <typename T>
inline void Expect<T>::expect(const char* const msg) const noexcept
{
    if (!(*static_cast<const T*>(this)))
    {
        std::cout << msg << std::endl;
        Ensures(false);
    }
}

template <typename T, typename ValueType>
inline ValueType& ExpectWithValue<T, ValueType>::expect(const char* const msg) & noexcept
{
    T* upcastedThis = static_cast<T*>(this);

    if (!(*upcastedThis))
    {
        std::cout << msg << std::endl;
        Ensures(false);
    }

    return upcastedThis->value();
}

template <typename T, typename ValueType>
inline const ValueType& ExpectWithValue<T, ValueType>::expect(const char* const msg) const& noexcept
{
    return const_cast<const ValueType&>(const_cast<ExpectWithValue<T, ValueType>*>(this)->expect(msg));
}

template <typename T, typename ValueType>
inline ValueType&& ExpectWithValue<T, ValueType>::expect(const char* const msg) && noexcept
{
    return std::move(this->expect(msg));
}

template <typename T, typename ValueType>
inline const ValueType&& ExpectWithValue<T, ValueType>::expect(const char* const msg) const&& noexcept
{
    return const_cast<const ValueType&&>(std::move(const_cast<ExpectWithValue<T, ValueType>*>(this)->expect(msg)));
}

template <typename T, typename ValueType>
inline ValueType ValueOr<T, ValueType>::value_or(const ValueType& value) const noexcept
{
    const T* upcastedThis = static_cast<const T*>(this);

    if (!(*upcastedThis))
    {
        return value;
    }

    return upcastedThis->value();
}

template <typename T, typename ValueType>
inline T& AndThenWithValue<T, ValueType>::and_then(const and_then_callback_t& callable) & noexcept
{
    T* upcastedThis = static_cast<T*>(this);

    if (*upcastedThis)
    {
        callable(upcastedThis->value());
    }

    return *upcastedThis;
}

template <typename T, typename ValueType>
inline T&& AndThenWithValue<T, ValueType>::and_then(const and_then_callback_t& callable) && noexcept
{
    return std::move(this->and_then(callable));
}

template <typename T, typename ValueType>
inline const T& AndThenWithValue<T, ValueType>::and_then(const const_and_then_callback_t& callable) const& noexcept
{
    const T* upcastedThis = static_cast<const T*>(this);

    if (*upcastedThis)
    {
        callable(upcastedThis->value());
    }

    return *upcastedThis;
}

template <typename T, typename ValueType>
inline const T&& AndThenWithValue<T, ValueType>::and_then(const const_and_then_callback_t& callable) const&& noexcept
{
    return std::move(this->and_then(callable));
}

template <typename T>
inline T& AndThen<T>::and_then(const and_then_callback_t& callable) & noexcept
{
    T* upcastedThis = static_cast<T*>(this);

    if (*upcastedThis)
    {
        callable();
    }

    return *upcastedThis;
}

template <typename T>
inline const T& AndThen<T>::and_then(const and_then_callback_t& callable) const& noexcept
{
    return const_cast<const T&>(const_cast<AndThen<T>*>(this)->and_then(callable));
}

template <typename T>
inline T&& AndThen<T>::and_then(const and_then_callback_t& callable) && noexcept
{
    return std::move(this->and_then(callable));
}

template <typename T>
inline const T&& AndThen<T>::and_then(const and_then_callback_t& callable) const&& noexcept
{
    return std::move(const_cast<const T&>(const_cast<AndThen<T>*>(this)->and_then(callable)));
}

template <typename T, typename ErrorType>
inline T& OrElseWithValue<T, ErrorType>::or_else(const or_else_callback_t& callable) & noexcept
{
    T* upcastedThis = static_cast<T*>(this);

    if (!(*upcastedThis))
    {
        callable(upcastedThis->get_error());
    }

    return *upcastedThis;
}

template <typename T, typename ErrorType>
inline T&& OrElseWithValue<T, ErrorType>::or_else(const or_else_callback_t& callable) && noexcept
{
    return std::move(this->or_else(callable));
}

template <typename T, typename ErrorType>
inline const T& OrElseWithValue<T, ErrorType>::or_else(const const_or_else_callback_t& callable) const& noexcept
{
    const T* upcastedThis = static_cast<const T*>(this);

    if (!(*upcastedThis))
    {
        callable(upcastedThis->get_error());
    }

    return *upcastedThis;
}

template <typename T, typename ErrorType>
inline const T&& OrElseWithValue<T, ErrorType>::or_else(const const_or_else_callback_t& callable) const&& noexcept
{
    return std::move(this->or_else(callable));
}

template <typename T>
inline T& OrElse<T>::or_else(const or_else_callback_t& callable) & noexcept
{
    T* upcastedThis = static_cast<T*>(this);

    if (!(*upcastedThis))
    {
        callable();
    }

    return *upcastedThis;
}

template <typename T>
inline T&& OrElse<T>::or_else(const or_else_callback_t& callable) && noexcept
{
    return std::move(this->or_else(callable));
}

template <typename T>
inline const T& OrElse<T>::or_else(const or_else_callback_t& callable) const& noexcept
{
    return const_cast<const T&>(const_cast<OrElse<T>*>(this)->or_else(callable));
}

template <typename T>
inline const T&& OrElse<T>::or_else(const or_else_callback_t& callable) const&& noexcept
{
    return std::move(this->or_else(callable));
}


} // namespace internal
} // namespace cxx
} // namespace iox
#endif
