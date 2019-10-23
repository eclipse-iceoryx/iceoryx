// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

namespace iox
{
namespace cxx
{
template <typename T>
inline optional<T>::optional(const nullopt_t&) noexcept
{
}

template <typename T>
inline optional<T>::optional() noexcept
    : optional(nullopt_t())
{
}

template <typename T>
inline optional<T>::optional(T&& value) noexcept
{
    construct_value(std::forward<T>(value));
}

template <typename T>
inline optional<T>::optional(const optional& rhs) noexcept
{
    if (rhs.m_value)
    {
        construct_value(*rhs.m_value);
    }
}

template <typename T>
inline optional<T>::optional(optional&& rhs) noexcept
{
    if (rhs.m_value)
    {
        construct_value(std::move(*rhs.m_value));
        rhs.destruct_value();
    }
}

template <typename T>
inline optional<T>& optional<T>::operator=(const optional& rhs) noexcept
{
    if (this != &rhs)
    {
        if (!rhs.m_value && m_value)
        {
            destruct_value();
        }
        else if (rhs.m_value && m_value)
        {
            *m_value = *rhs.m_value;
        }
        else if (rhs.m_value && !m_value)
        {
            construct_value(*rhs.m_value);
        }
    }
    return *this;
}

template <typename T>
inline optional<T>& optional<T>::operator=(optional&& rhs) noexcept
{
    if (this != &rhs)
    {
        if (!rhs.m_value && m_value)
        {
            destruct_value();
        }
        else if (rhs.m_value && m_value)
        {
            *m_value = std::move(*rhs.m_value);
        }
        else if (rhs.m_value && !m_value)
        {
            construct_value(std::move(*rhs.m_value));
        }
        if (rhs.m_value)
        {
            rhs.destruct_value();
        }
    }
    return *this;
}

template <typename T>
inline optional<T>::~optional() noexcept
{
    if (m_value)
    {
        destruct_value();
    }
}

template <typename T>
template <typename U>
inline typename std::enable_if<!std::is_same<U, optional<T>&>::value, optional<T>>::type& optional<T>::
operator=(U&& value) noexcept
{
    if (m_value)
    {
        *m_value = std::forward<T>(value);
    }
    else
    {
        construct_value(std::forward<T>(value));
    }
    return *this;
}

template <typename T>
constexpr inline bool optional<T>::operator==(const optional<T>& rhs) const noexcept
{
    return (!m_value && !rhs.m_value) || ((m_value && rhs.m_value) && (*m_value == *rhs.m_value));
}

template <typename T>
constexpr inline bool optional<T>::operator==(const nullopt_t&) const noexcept
{
    return !m_value;
}

template <typename T>
constexpr inline bool optional<T>::operator!=(const optional<T>& rhs) const noexcept
{
    return !(*this == rhs);
}

template <typename T>
constexpr inline bool optional<T>::operator!=(const nullopt_t& rhs) const noexcept
{
    return !(*this == rhs);
}

template <typename T>
inline const T* optional<T>::operator->() const noexcept
{
    return m_value;
}

template <typename T>
inline const T& optional<T>::operator*() const noexcept
{
    return *m_value;
}

template <typename T>
inline T* optional<T>::operator->() noexcept
{
    return const_cast<T*>(const_cast<const optional*>(this)->operator->());
}

template <typename T>
inline T& optional<T>::operator*() noexcept
{
    return const_cast<T&>(const_cast<const optional*>(this)->operator*());
}

template <typename T>
inline constexpr optional<T>::operator bool() const noexcept
{
    return m_value;
}

template <typename T>
template <typename... Targs>
inline T& optional<T>::emplace(Targs&&... args) noexcept
{
    if (m_value)
    {
        destruct_value();
    }

    construct_value(std::forward<Targs>(args)...);
    return *m_value;
}

template <typename T>
inline constexpr bool optional<T>::has_value() const noexcept
{
    return m_value;
}

template <typename T>
inline void optional<T>::reset() noexcept
{
    if (m_value)
    {
        destruct_value();
    }
}

template <typename T>
    inline T& optional<T>::value() & noexcept
{
    return this->operator*();
}

template <typename T>
inline const T& optional<T>::value() const& noexcept
{
    return this->operator*();
}

template <typename T>
    inline T&& optional<T>::value() && noexcept
{
    return std::move(*m_value);
}

template <typename T>
inline const T&& optional<T>::value() const&& noexcept
{
    return std::move(*m_value);
}


template <typename T>
template <typename U>
inline constexpr T optional<T>::value_or(U&& default_value) const noexcept
{
    return (m_value) ? *m_value : std::forward<U>(default_value);
}

template <typename T>
template <typename... Targs>
inline void optional<T>::construct_value(Targs&&... args) noexcept
{
    m_value = new (m_data) T(std::forward<Targs>(args)...);
}

template <typename T>
inline void optional<T>::destruct_value() noexcept
{
    m_value->~T();
    m_value = nullptr;
}

template <typename OptionalBaseType, typename... Targs>
inline optional<OptionalBaseType> make_optional(Targs&&... args) noexcept
{
    optional<OptionalBaseType> returnValue = nullopt_t();
    returnValue.emplace(std::forward<Targs>(args)...);
    return returnValue;
}
} // namespace cxx
} // namespace iox
