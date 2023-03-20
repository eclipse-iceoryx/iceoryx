// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_VOCABULARY_SEMANTIC_STRING_INL
#define IOX_HOOFS_VOCABULARY_SEMANTIC_STRING_INL

#include "iox/logging.hpp"
#include "iox/semantic_string.hpp"

namespace iox
{

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
template <uint64_t N>
inline SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::SemanticString(
    const string<N>& value) noexcept
    : m_data{value}
{
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
template <uint64_t N>
inline expected<SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>,
                SemanticStringError>
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::create_impl(
    const char* value) noexcept
{
    if (N > Capacity)
    {
        IOX_LOG(WARN) << "Unable to create semantic string since the value \"" << value
                      << "\" exceeds the maximum valid length of " << Capacity << ".";
        return iox::error<SemanticStringError>(SemanticStringError::ExceedsMaximumLength);
    }

    auto str = string<Capacity>(TruncateToCapacity, value);

    if (DoesContainInvalidCharacterCall(str))
    {
        IOX_LOG(WARN) << "Unable to create semantic string since the value \"" << value
                      << "\" contains invalid characters";
        return iox::error<SemanticStringError>(SemanticStringError::ContainsInvalidCharacters);
    }

    if (DoesContainInvalidContentCall(str))
    {
        IOX_LOG(WARN) << "Unable to create semantic string since the value \"" << value
                      << "\" contains invalid content";
        return iox::error<SemanticStringError>(SemanticStringError::ContainsInvalidContent);
    }

    return iox::success<SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>>(
        SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>(str));
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
template <uint64_t N>
inline expected<SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>,
                SemanticStringError>
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::create(
    // avoid-c-arrays: justification in header
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)
    const char (&value)[N]) noexcept
{
    return SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::
        template create_impl<N>(value);
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
template <uint64_t N>
inline expected<SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>,
                SemanticStringError>
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::create(
    const string<N>& value) noexcept
{
    return SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::
        template create_impl<N>(value.c_str());
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
inline constexpr uint64_t
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::size() const noexcept
{
    return m_data.size();
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
inline constexpr uint64_t
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::capacity() noexcept
{
    return Capacity;
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
inline constexpr const string<Capacity>&
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::as_string() const noexcept
{
    return m_data;
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
template <typename T>
inline expected<SemanticStringError>
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::append(
    const T& value) noexcept
{
    return insert(size(), value, value.size());
}

template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
template <typename T>
inline expected<SemanticStringError>
SemanticString<Capacity, DoesContainInvalidContentCall, DoesContainInvalidCharacterCall>::insert(
    const uint64_t pos, const T& str, const uint64_t count) noexcept
{
    auto temp = m_data;
    if (!temp.insert(pos, str, count))
    {
        IOX_LOG(WARN) << "Unable to insert the value \"" << str
                      << "\" to the semantic string since it would exceeds the maximum valid length of " << Capacity
                      << ".";
        return iox::error<SemanticStringError>(SemanticStringError::ExceedsMaximumLength);
    }

    if (DoesContainInvalidCharacterCall(temp))
    {
        IOX_LOG(WARN) << "Unable to insert the value \"" << str
                      << "\" to the semantic string since it contains invalid characters.";
        return iox::error<SemanticStringError>(SemanticStringError::ContainsInvalidCharacters);
    }

    if (DoesContainInvalidContentCall(str))
    {
        IOX_LOG(WARN) << "Unable to insert the value \"" << str
                      << "\" to the semantic string since it would lead to invalid content.";
        return iox::error<SemanticStringError>(SemanticStringError::ContainsInvalidContent);
    }

    m_data = temp;
    return iox::success<>();
}

} // namespace iox

#endif
