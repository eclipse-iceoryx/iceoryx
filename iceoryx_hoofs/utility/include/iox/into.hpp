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
#ifndef IOX_HOOFS_UTILITY_INTO_HPP
#define IOX_HOOFS_UTILITY_INTO_HPP

namespace iox
{

template <typename D>
struct lossy
{
};
namespace detail
{
template <typename T>
struct extract_into_type
{
    using type_t = T;
};


template <typename T>
struct extract_into_type<lossy<T>>
{
    using type_t = T;
};
} // namespace detail

/// @brief Converts a value of type SourceType to a corresponding value of type DestinationType. This function needs to
/// be specialized by the user for the types to be converted.
/// @code
/// enum class LowLevel
/// {
///     FileDescriptorInvalid,
///     FileDescriptorCorrupt,
///     Timeout
/// };
///
/// enum class HighLevel
/// {
///     FileDescriptorError,
///     Timeout
/// };
///
/// namespace iox
/// {
/// namespace cxx
/// {
/// template <>
/// constexpr HighLevel from<LowLevel, HighLevel>(LowLevel e) noexcept
/// {
///     switch (e)
///     {
///     case LowLevel::FileDescriptorCorrupt:
///         return HighLevel::FileDescriptorError;
///     case LowLevel::FileDescriptorInvalid:
///         return HighLevel::FileDescriptorError;
///     case LowLevel::Timeout:
///         return HighLevel::Timeout;
///     }
/// }
/// } // namespace cxx
/// } // namespace iox
/// @endcode
/// @tparam SourceType is the 'from' type
/// @tparam DestinationType is the 'to' type
/// @param[in] value of type SourceType to convert to DestinationType
/// @return converted value of SourceType to corresponding value of DestinationType
template <typename SourceType, typename DestinationType>
constexpr typename detail::extract_into_type<DestinationType>::type_t from(const SourceType value);


// Using a struct as impl, as free functions do not support partially specialized templates
template <typename SourceType, typename DestinationType>
struct FromImpl
{
    static auto fromImpl(const SourceType& value);
};

/// @brief Converts a value of type SourceType to a corresponding value of type DestinationType. This is a convenience
/// function which is automatically available when `from` is implemented. This function shall therefore not be
/// specialized but always the `from` function.
/// @code
/// Bar b = iox::into<Bar>(Foo::ENUM_VALUE);
/// @endcode
/// @tparam DestinationType is the 'to' type
/// @tparam SourceType is the 'from' type
/// @param[in] value of type SourceType to convert to DestinationType
/// @return converted value of SourceType to corresponding value of DestinationType
template <typename DestinationType, typename SourceType>
constexpr typename detail::extract_into_type<DestinationType>::type_t into(const SourceType value);

} // namespace iox

#include "iox/detail/into.inl"

#endif // IOX_HOOFS_UTILITY_INTO_HPP
