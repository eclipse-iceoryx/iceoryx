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
inline success<T>::success(const T& t) noexcept
    : value(t)
{
}

template <typename T>
inline success<T>::success(T&& t) noexcept
    : value(std::forward<T>(t))
{
}

template <typename T>
template <typename... Targs>
inline success<T>::success(Targs&&... args) noexcept
    : value(std::forward<Targs>(args)...)
{
}

template <typename T>
inline error<T>::error(const T& t) noexcept
    : value(t)
{
}

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


template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(variant<ValueType, ErrorType>&& f_store, const bool hasError) noexcept
    : m_store(std::move(f_store))
    , m_hasError(hasError)
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(const success<ValueType>& successValue) noexcept
    : m_store(in_place_index<0>(), successValue.value)
    , m_hasError(false)
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(success<ValueType>&& successValue) noexcept
    : m_store(in_place_index<0>(), std::move(successValue.value))
    , m_hasError(false)
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(const error<ErrorType>& errorValue) noexcept
    : m_store(in_place_index<1>(), errorValue.value)
    , m_hasError(true)
{
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>::expected(error<ErrorType>&& errorValue) noexcept
    : m_store(in_place_index<1>(), std::move(errorValue.value))
    , m_hasError(true)
{
}

template <typename ValueType, typename ErrorType>
template <typename... Targs>
inline expected<ValueType, ErrorType> expected<ValueType, ErrorType>::create_value(Targs&&... args) noexcept
{
    expected<ValueType, ErrorType> returnValue(
        variant<ValueType, ErrorType>(in_place_index<0>(), std::forward<Targs>(args)...), false);

    return returnValue;
}

template <typename ValueType, typename ErrorType>
template <typename... Targs>
inline expected<ValueType, ErrorType> expected<ValueType, ErrorType>::create_error(Targs&&... args) noexcept
{
    expected<ValueType, ErrorType> returnValue(
        variant<ValueType, ErrorType>(in_place_index<1>(), std::forward<Targs>(args)...), true);

    return returnValue;
}

template <typename ValueType, typename ErrorType>
inline bool expected<ValueType, ErrorType>::has_error() const noexcept
{
    return m_hasError;
}

template <typename ValueType, typename ErrorType>
inline const ErrorType&& expected<ValueType, ErrorType>::get_error() const&& noexcept
{
    return std::move(*m_store.template get_at_index<1>());
}

template <typename ValueType, typename ErrorType>
    inline ErrorType&& expected<ValueType, ErrorType>::get_error() && noexcept
{
    return std::move(*m_store.template get_at_index<1>());
}

template <typename ValueType, typename ErrorType>
    inline ErrorType& expected<ValueType, ErrorType>::get_error() & noexcept
{
    return *m_store.template get_at_index<1>();
}

template <typename ValueType, typename ErrorType>
inline const ErrorType& expected<ValueType, ErrorType>::get_error() const& noexcept
{
    return *m_store.template get_at_index<1>();
}

template <typename ValueType, typename ErrorType>
    inline ValueType&& expected<ValueType, ErrorType>::get_value() && noexcept
{
    return std::move(*m_store.template get_at_index<0>());
}

template <typename ValueType, typename ErrorType>
inline const ValueType& expected<ValueType, ErrorType>::get_value() const& noexcept
{
    return *m_store.template get_at_index<0>();
}

template <typename ValueType, typename ErrorType>
inline const ValueType&& expected<ValueType, ErrorType>::get_value() const&& noexcept
{
    return std::move(*m_store.template get_at_index<0>());
}

template <typename ValueType, typename ErrorType>
    inline ValueType& expected<ValueType, ErrorType>::get_value() & noexcept
{
    return *m_store.template get_at_index<0>();
}

template <typename ValueType, typename ErrorType>
inline ValueType expected<ValueType, ErrorType>::get_value_or(const ValueType& value) noexcept
{
    if (this->has_error())
    {
        return value;
    }

    return *m_store.template get_at_index<0>();
}

template <typename ValueType, typename ErrorType>
inline ValueType expected<ValueType, ErrorType>::get_value_or(const ValueType& value) const noexcept
{
    return const_cast<expected*>(this)->get_value_or(value);
}

template <typename ValueType, typename ErrorType>
inline ValueType* expected<ValueType, ErrorType>::operator->() noexcept
{
    return m_store.template get_at_index<0>();
}

template <typename ValueType, typename ErrorType>
inline const ValueType* expected<ValueType, ErrorType>::operator->() const noexcept
{
    return const_cast<expected*>(this)->operator->();
}

template <typename ValueType, typename ErrorType>
inline ValueType& expected<ValueType, ErrorType>::operator*() noexcept
{
    return *m_store.template get_at_index<0>();
}

template <typename ValueType, typename ErrorType>
inline const ValueType& expected<ValueType, ErrorType>::operator*() const noexcept
{
    return const_cast<expected*>(this)->operator*();
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_error(const std::function<void(expected&)>& callable) noexcept
{
    if (this->has_error())
    {
        callable(*this);
    }

    return *this;
}

template <typename ValueType, typename ErrorType>
inline const expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_error(const std::function<void(expected&)>& callable) const noexcept
{
    return const_cast<expected*>(this)->on_error(callable);
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_error(const std::function<void()>& callable) noexcept
{
    if (this->has_error())
    {
        callable();
    }

    return *this;
}

template <typename ValueType, typename ErrorType>
inline const expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_error(const std::function<void()>& callable) const noexcept
{
    return const_cast<expected*>(this)->on_error(callable);
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_success(const std::function<void(expected&)>& callable) noexcept
{
    if (!this->has_error())
    {
        callable(*this);
    }

    return *this;
}

template <typename ValueType, typename ErrorType>
inline const expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_success(const std::function<void(expected&)>& callable) const noexcept
{
    return const_cast<expected*>(this)->on_success(callable);
}

template <typename ValueType, typename ErrorType>
inline expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_success(const std::function<void()>& callable) noexcept
{
    if (!this->has_error())
    {
        callable();
    }

    return *this;
}

template <typename ValueType, typename ErrorType>
inline const expected<ValueType, ErrorType>&
expected<ValueType, ErrorType>::on_success(const std::function<void()>& callable) const noexcept
{
    return const_cast<expected*>(this)->on_success(callable);
}

template <typename ValueType, typename ErrorType>
template <typename T>
inline expected<ValueType, ErrorType>::operator expected<T>() noexcept
{
    if (this->has_error())
        return error<ErrorType>(this->get_error());
    else
        return success<>();
}

template <typename ValueType, typename ErrorType>
template <typename T>
inline expected<ValueType, ErrorType>::operator expected<T>() const noexcept
{
    return const_cast<expected*>(this)->operator expected<T>();
}

// expected<ErrorType>

template <typename ErrorType>
inline expected<ErrorType>::expected(variant<ErrorType>&& f_store, const bool hasError) noexcept
    : m_store(std::move(f_store))
    , m_hasError(hasError)
{
}

template <typename ErrorType>
inline expected<ErrorType>::expected(const success<void>&) noexcept
    : m_hasError(false)
{
}

template <typename ErrorType>
inline expected<ErrorType>::expected(const error<ErrorType>& errorValue) noexcept
    : m_store(in_place_index<0>(), errorValue.value)
    , m_hasError(true)
{
}

template <typename ErrorType>
inline expected<ErrorType>::expected(error<ErrorType>&& errorValue) noexcept
    : m_store(in_place_index<0>(), std::move(errorValue.value))
    , m_hasError(true)
{
}

template <typename ErrorType>
inline expected<ErrorType> expected<ErrorType>::create_value() noexcept
{
    expected<ErrorType> returnValue(variant<ErrorType>(), false);

    return returnValue;
}

template <typename ErrorType>
template <typename... Targs>
inline expected<ErrorType> expected<ErrorType>::create_error(Targs&&... args) noexcept
{
    expected<ErrorType> returnValue(variant<ErrorType>(in_place_index<0>(), std::forward<Targs>(args)...), true);

    return returnValue;
}

template <typename ErrorType>
inline bool expected<ErrorType>::has_error() const noexcept
{
    return m_hasError;
}

template <typename ErrorType>
    inline ErrorType&& expected<ErrorType>::get_error() && noexcept
{
    return std::move(*m_store.template get_at_index<0>());
}

template <typename ErrorType>
inline const ErrorType& expected<ErrorType>::get_error() const& noexcept
{
    return *m_store.template get_at_index<0>();
}

template <typename ErrorType>
inline const ErrorType&& expected<ErrorType>::get_error() const&& noexcept
{
    return std::move(*m_store.template get_at_index<0>());
}

template <typename ErrorType>
    inline ErrorType& expected<ErrorType>::get_error() & noexcept
{
    return *m_store.template get_at_index<0>();
}

template <typename ErrorType>
inline expected<ErrorType>& expected<ErrorType>::on_error(const std::function<void(expected&)>& callable) noexcept
{
    if (this->has_error())
    {
        callable(*this);
    }

    return *this;
}

template <typename ErrorType>
inline const expected<ErrorType>& expected<ErrorType>::on_error(const std::function<void(expected&)>& callable) const
    noexcept
{
    return const_cast<expected*>(this)->on_error(callable);
}

template <typename ErrorType>
inline expected<ErrorType>& expected<ErrorType>::on_error(const std::function<void()>& callable) noexcept
{
    if (this->has_error())
    {
        callable();
    }

    return *this;
}

template <typename ErrorType>
inline const expected<ErrorType>& expected<ErrorType>::on_error(const std::function<void()>& callable) const noexcept
{
    return const_cast<expected*>(this)->on_error(callable);
}

template <typename ErrorType>
inline expected<ErrorType>& expected<ErrorType>::on_success(const std::function<void(expected&)>& callable) noexcept
{
    if (!this->has_error())
    {
        callable(*this);
    }

    return *this;
}

template <typename ErrorType>
inline const expected<ErrorType>& expected<ErrorType>::on_success(const std::function<void(expected&)>& callable) const
    noexcept
{
    return const_cast<expected*>(this)->on_success(callable);
}

template <typename ErrorType>
inline expected<ErrorType>& expected<ErrorType>::on_success(const std::function<void()>& callable) noexcept
{
    if (!this->has_error())
    {
        callable();
    }

    return *this;
}

template <typename ErrorType>
inline const expected<ErrorType>& expected<ErrorType>::on_success(const std::function<void()>& callable) const noexcept
{
    return const_cast<expected*>(this)->on_success(callable);
}
} // namespace cxx
} // namespace iox
