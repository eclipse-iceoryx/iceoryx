// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_VOCABULARY_EXPECTED_INL
#define IOX_HOOFS_VOCABULARY_EXPECTED_INL

#include "iox/expected.hpp"

namespace iox
{
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : This is a false positive since there is no fitting constructor
// available for delegation
template <typename T>
inline success<T>::success(const T& t) noexcept
    : value(t)
{
}

// AXIVION Next Construct AutosarC++19_03-A18.9.2 : For universal references std::forward must be used
template <typename T>
inline success<T>::success(T&& t) noexcept
    : value(std::forward<T>(t))
{
}

// AXIVION Next Construct AutosarC++19_03-A15.4.2, FaultDetection-NoexceptViolations : Intentional behavior. 'success' is not intended to be used with a type which throws
template <typename T>
template <typename... Targs>
inline success<T>::success(Targs&&... args) noexcept
    : value(std::forward<Targs>(args)...)
{
}
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : This is a false positive since there is no fitting constructor
// available for delegation
template <typename T>
inline error<T>::error(const T& t) noexcept
    : value(t)
{
}
// AXIVION Next Construct AutosarC++19_03-A18.9.2 : For universal references std::forward must be used
template <typename T>
inline error<T>::error(T&& t) noexcept
    : value(std::forward<T>(t))
{
}

template <typename T>
template <typename... Targs>
inline error<T>::error(Targs&&... args) noexcept
    : value(std::forward<Targs>(args)...)
{
}
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : This ctor uses a rvalue reference of the underlying type which
// makes it similar to the move ctor and therefore a delegating ctor cannot be used
template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(variant<ValueType, ErrorType>&& store) noexcept
    : m_store(std::move(store))
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(const success<ValueType>& successValue) noexcept
    : m_store(in_place_index<VALUE_INDEX>(), successValue.value)
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(success<ValueType>&& successValue) noexcept
    : m_store(in_place_index<VALUE_INDEX>(), std::move(successValue.value))
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(const error<ErrorType>& errorValue) noexcept
    : m_store(in_place_index<ERROR_INDEX>(), errorValue.value)
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(error<ErrorType>&& errorValue) noexcept
    : m_store(in_place_index<ERROR_INDEX>(), std::move(errorValue.value))
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(expected<ValueType, ErrorType>&& rhs) noexcept
    : m_store{std::move(rhs.m_store)}
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::operator=(expected<ValueType, ErrorType>&& rhs) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-M0.1.2, AutosarC++19_03-M0.1.9, FaultDetection-DeadBranches : False positive. Check needed to avoid self assignment.
    if (this != &rhs)
    {
        m_store = std::move(rhs.m_store);
    }
    return *this;
}

template <typename ValueType, typename ErrorType>
template <typename... Targs>
inline expected<ValueType, ErrorType> expected<ValueType, ErrorType>::create_value(Targs&&... args) noexcept
{
    expected<ValueType, ErrorType> returnValue{
        variant<ValueType, ErrorType>(in_place_index<VALUE_INDEX>(), std::forward<Targs>(args)...)};

    return returnValue;
}

template <typename ValueType, typename ErrorType>
template <typename... Targs>
inline expected<ValueType, ErrorType> expected<ValueType, ErrorType>::create_error(Targs&&... args) noexcept
{
    expected<ValueType, ErrorType> returnValue{
        variant<ValueType, ErrorType>(in_place_index<ERROR_INDEX>(), std::forward<Targs>(args)...)};

    return returnValue;
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::operator bool() const noexcept
{
    return !has_error();
}

template <typename ValueType, typename ErrorType>
inline bool expected<ValueType, ErrorType>::has_error() const noexcept
{
    return m_store.index() == ERROR_INDEX;
}

template <typename ValueType, typename ErrorType>
inline ErrorType&& expected<ValueType, ErrorType>::get_error() && noexcept
{
    return std::move(get_error());
}

template <typename ValueType, typename ErrorType>
inline ErrorType& expected<ValueType, ErrorType>::get_error() & noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<ErrorType&>(const_cast<const expected<ValueType, ErrorType>*>(this)->get_error());
}

template <typename ValueType, typename ErrorType>
inline const ErrorType& expected<ValueType, ErrorType>::get_error() const& noexcept
{
    cxx::ExpectsWithMsg(has_error(), "Trying to access an error but a value is stored!");
    return get_error_unchecked();
}

template <typename ValueType, typename ErrorType>
inline const ErrorType& expected<ValueType, ErrorType>::get_error_unchecked() const noexcept
{
    return *m_store.template get_at_index<ERROR_INDEX>();
}

template <typename ValueType, typename ErrorType>
inline ValueType&& expected<ValueType, ErrorType>::value() && noexcept
{
    return std::move(value());
}

template <typename ValueType, typename ErrorType>
inline const ValueType& expected<ValueType, ErrorType>::value() const& noexcept
{
    cxx::ExpectsWithMsg(!has_error(), "Trying to access a value but an error is stored!");
    return value_unchecked();
}

template <typename ValueType, typename ErrorType>
inline ValueType& expected<ValueType, ErrorType>::value() & noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<ValueType&>(const_cast<const expected<ValueType, ErrorType>*>(this)->value());
}

template <typename ValueType, typename ErrorType>
inline ValueType* expected<ValueType, ErrorType>::operator->() noexcept
{
    return &value();
}

template <typename ValueType, typename ErrorType>
inline const ValueType* expected<ValueType, ErrorType>::operator->() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const_cast avoids code duplication, is safe since the
    // constness of the return value is restored
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<const ValueType*>(const_cast<expected*>(this)->operator->());
}

template <typename ValueType, typename ErrorType>
inline ValueType& expected<ValueType, ErrorType>::operator*() noexcept
{
    return value();
}

template <typename ValueType, typename ErrorType>
inline const ValueType& expected<ValueType, ErrorType>::operator*() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const_cast avoids code duplication and is safe here, since the
    // constness of the return value is restored
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<const ValueType&>(const_cast<expected*>(this)->operator*());
}

template <typename ValueType, typename ErrorType>
inline const ValueType& expected<ValueType, ErrorType>::value_unchecked() const noexcept
{
    return *m_store.template get_at_index<VALUE_INDEX>();
}

// AXIVION Next Construct AutosarC++19_03-A13.5.2, AutosarC++19_03-A13.5.3: see doxygen brief section in header
template <typename ValueType, typename ErrorType>
template <typename T>
inline expected<ValueType, ErrorType>::operator expected<T>() const noexcept
{
    if (has_error())
    {
        return error<ErrorType>(get_error());
    }
    return success<>();
}

template <typename ValueType, typename ErrorType>
inline optional<ValueType> expected<ValueType, ErrorType>::to_optional() const noexcept
{
    optional<ValueType> returnValue;
    if (!has_error())
    {
        returnValue.emplace(value());
    }
    return returnValue;
}
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : This ctor uses a rvalue reference of the underlying type which
// makes it similar to the move ctor and therefore a delegating ctor cannot be used
template <typename ErrorType>
inline expected<ErrorType>::expected(variant<ErrorType>&& store) noexcept
    : m_store(std::move(store))
{
}

template <typename ErrorType>
inline expected<ErrorType>::expected(const success<void>) noexcept
{
}

template <typename ErrorType>
inline expected<ErrorType>::expected(expected<ErrorType>&& rhs) noexcept
    : m_store(std::move(rhs.m_store))
{
}

template <typename ErrorType>
inline expected<ErrorType>& expected<ErrorType>::operator=(expected<ErrorType>&& rhs) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-M0.1.2, AutosarC++19_03-M0.1.9, FaultDetection-DeadBranches : False positive. Check needed to avoid self assignment.
    if (this != &rhs)
    {
        m_store = std::move(rhs.m_store);
    }
    return *this;
}

template <typename ErrorType>
inline expected<ErrorType>::expected(const error<ErrorType>& errorValue) noexcept
    : m_store(in_place_index<ERROR_INDEX>(), errorValue.value)
{
}

template <typename ErrorType>
inline expected<ErrorType>::expected(error<ErrorType>&& errorValue) noexcept
    : m_store(in_place_index<ERROR_INDEX>(), std::move(errorValue.value))
{
}
// AXIVION DISABLE STYLE AutosarC++19_03-A16.0.1: Required for Windows due to MSVC deficiencies
#if defined(_WIN32)
template <typename ErrorType>
template <typename ValueType>
inline expected<ErrorType>::expected(const expected<ValueType, ErrorType>& rhs) noexcept
{
    if (rhs.has_error())
    {
        m_store.emplace_at_index<ERROR_INDEX>(rhs.get_error());
    }
}

template <typename ErrorType>
template <typename ValueType>
inline expected<ErrorType>::expected(expected<ValueType, ErrorType>&& rhs) noexcept
{
    if (rhs.has_error())
    {
        m_store.emplace_at_index<ERROR_INDEX>(std::move(rhs.get_error()));
    }
}

template <typename ErrorType>
template <typename ValueType>
inline expected<ErrorType>& expected<ErrorType>::operator=(const expected<ValueType, ErrorType>& rhs) noexcept
{
    if (has_error() && rhs.has_error())
    {
        m_store.get_error() = rhs.get_error();
    }
    else if (rhs.has_error())
    {
        m_store = variant<ErrorType>(in_place_type<ErrorType>(), rhs.get_error());
    }
}

template <typename ErrorType>
template <typename ValueType>
inline expected<ErrorType>& expected<ErrorType>::operator=(expected<ValueType, ErrorType>&& rhs) noexcept
{
    if (has_error() && rhs.has_error())
    {
        m_store.get_error() = std::move(rhs.get_error());
    }
    else if (rhs.has_error())
    {
        m_store = variant<ErrorType>(in_place_type<ErrorType>(), std::move(rhs.get_error()));
    }
}
#endif
// AXIVION ENABLE STYLE AutosarC++19_03-A16.0.1

template <typename ErrorType>
inline expected<ErrorType> expected<ErrorType>::create_value() noexcept
{
    expected<ErrorType> returnValue{variant<ErrorType>()};

    return returnValue;
}

template <typename ErrorType>
template <typename... Targs>
inline expected<ErrorType> expected<ErrorType>::create_error(Targs&&... args) noexcept
{
    return static_cast<expected<ErrorType>>(
        variant<ErrorType>(in_place_index<ERROR_INDEX>(), std::forward<Targs>(args)...));
}

template <typename ErrorType>
inline expected<ErrorType>::operator bool() const noexcept
{
    return !has_error();
}

template <typename ErrorType>
inline bool expected<ErrorType>::has_error() const noexcept
{
    return (m_store.index() == ERROR_INDEX);
}

template <typename ErrorType>
inline ErrorType&& expected<ErrorType>::get_error() && noexcept
{
    return std::move(get_error());
}

template <typename ErrorType>
inline ErrorType& expected<ErrorType>::get_error() & noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<ErrorType&>(const_cast<const expected<ErrorType>*>(this)->get_error());
}

template <typename ErrorType>
inline const ErrorType& expected<ErrorType>::get_error() const& noexcept
{
    cxx::ExpectsWithMsg(has_error(), "Trying to access an error but a value is stored!");
    return get_error_unchecked();
}

template <typename ErrorType>
inline const ErrorType& expected<ErrorType>::get_error_unchecked() const noexcept
{
    return *m_store.template get_at_index<ERROR_INDEX>();
}

template <typename ErrorType>
inline constexpr bool operator==(const expected<ErrorType>& lhs, const expected<ErrorType>& rhs) noexcept
{
    if (lhs.has_error() != rhs.has_error())
    {
        return false;
    }
    if (lhs.has_error() && rhs.has_error())
    {
        return lhs.get_error() == rhs.get_error();
    }
    return true;
}

template <typename ErrorType>
inline constexpr bool operator!=(const expected<ErrorType>& lhs, const expected<ErrorType>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename ValueType, typename ErrorType>
inline constexpr bool operator==(const expected<ValueType, ErrorType>& lhs,
                                 const expected<ValueType, ErrorType>& rhs) noexcept
{
    if (lhs.has_error() != rhs.has_error())
    {
        return false;
    }
    if (lhs.has_error() && rhs.has_error())
    {
        return lhs.get_error() == rhs.get_error();
    }
    return lhs.value() == rhs.value();
}

template <typename ValueType, typename ErrorType>
inline constexpr bool operator!=(const expected<ValueType, ErrorType>& lhs,
                                 const expected<ValueType, ErrorType>& rhs) noexcept
{
    return !(lhs == rhs);
}
} // namespace iox

#endif // IOX_HOOFS_VOCABULARY_EXPECTED_INL
