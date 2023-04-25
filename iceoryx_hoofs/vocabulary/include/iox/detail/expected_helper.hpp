// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_VOCABULARY_EXPECTED_HELPER_HPP
#define IOX_HOOFS_VOCABULARY_EXPECTED_HELPER_HPP

#include "iox/optional.hpp"
#include "iox/variant.hpp"

namespace iox
{
/// @brief helper struct which is used to call the in-place-construction constructor for error types
struct unexpect_t
{
};
constexpr unexpect_t unexpect{};

/// @brief forward declaration required for 'compare_expected_value'
template <typename ValueType, typename ErrorType>
class expected;

namespace detail
{
/// @brief helper class to be able to handle 'void' value type specialization
template <typename ValueType, typename ErrorType>
class expected_storage
{
  public:
    expected_storage() noexcept = delete;

    template <typename... Targs>
    explicit expected_storage(in_place_t, Targs&&... args)
        : data(in_place_index<VALUE_INDEX>(), std::forward<Targs>(args)...)
    {
    }

    template <typename... Targs>
    explicit expected_storage(unexpect_t, Targs&&... args)
        : data(in_place_index<ERROR_INDEX>(), std::forward<Targs>(args)...)
    {
    }

    bool has_value() const
    {
        return data.index() == VALUE_INDEX;
    }

    bool has_error() const
    {
        return data.index() == ERROR_INDEX;
    }

    ValueType& value_unchecked()
    {
        return *data.template get_at_index<VALUE_INDEX>();
    }

    const ValueType& value_unchecked() const
    {
        return *data.template get_at_index<VALUE_INDEX>();
    }

    ErrorType& error_unchecked()
    {
        return *data.template get_at_index<ERROR_INDEX>();
    }

    const ErrorType& error_unchecked() const
    {
        return *data.template get_at_index<ERROR_INDEX>();
    }

  private:
    static constexpr uint64_t VALUE_INDEX{0};
    static constexpr uint64_t ERROR_INDEX{1};

    iox::variant<ValueType, ErrorType> data;
};

template <typename ErrorType>
class expected_storage<void, ErrorType>
{
  public:
    expected_storage() noexcept = delete;

    template <typename... Targs>
    explicit expected_storage(in_place_t, Targs&&...)
        : data(in_place_index<VALUE_INDEX>(), DUMMY_VALUE)
    {
    }

    template <typename... Targs>
    explicit expected_storage(unexpect_t, Targs&&... args)
        : data(in_place_index<ERROR_INDEX>(), std::forward<Targs>(args)...)
    {
    }

    bool has_value() const
    {
        return data.index() == VALUE_INDEX;
    }

    bool has_error() const
    {
        return data.index() == ERROR_INDEX;
    }

    void value_unchecked() const
    {
        // nothing to do
    }

    ErrorType& error_unchecked()
    {
        return *data.template get_at_index<ERROR_INDEX>();
    }

    const ErrorType& error_unchecked() const
    {
        return *data.template get_at_index<ERROR_INDEX>();
    }

  private:
    static constexpr uint64_t VALUE_INDEX{0};
    static constexpr uint64_t ERROR_INDEX{1};

    using DummyValueType = bool;
    static constexpr DummyValueType DUMMY_VALUE{true};

    iox::variant<DummyValueType, ErrorType> data;
};

template <typename ErrorType>
constexpr typename expected_storage<void, ErrorType>::DummyValueType expected_storage<void, ErrorType>::DUMMY_VALUE;

/// @brief helper struct for 'operator==' to be able to handle 'void' value type specialization
template <typename T, typename E>
struct compare_expected_value
{
    static constexpr bool is_same_value_unchecked(const expected_storage<T, E>& lhs, const expected_storage<T, E>& rhs)
    {
        return lhs.value_unchecked() == rhs.value_unchecked();
    }
};

template <typename E>
struct compare_expected_value<void, E>
{
    static constexpr bool is_same_value_unchecked(const expected_storage<void, E>&, const expected_storage<void, E>&)
    {
        return true;
    }
};

} // namespace detail
} // namespace iox

#endif // IOX_HOOFS_VOCABULARY_EXPECTED_HELPER_HPP
