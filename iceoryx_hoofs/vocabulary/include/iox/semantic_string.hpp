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

#ifndef IOX_HOOFS_VOCABULARY_SEMANTIC_STRING_HPP
#define IOX_HOOFS_VOCABULARY_SEMANTIC_STRING_HPP

#include "iox/expected.hpp"
#include "iox/string.hpp"

#include <cstdint>

namespace iox
{
/// @brief Defines errors which can occur when modifying or creating a
///         SemanticString
enum class SemanticStringError
{
    ContainsInvalidCharacters,
    ContainsInvalidContent,
    ExceedsMaximumLength
};

template <uint64_t Capacity>
using DoesContainInvalidCharacter = bool (*)(const string<Capacity>& value);

template <uint64_t Capacity>
using DoesContainInvalidContent = bool (*)(const string<Capacity>& value);

/// @brief The SemanticString is a string which as an inner syntax and restrictions
///         to valid content. Examples are for instance
///         * UserName, only characters and numbers are allowed
///         * FileNames, no slashes etc.
///        The SemanticString shall be a basic building block to create string
///        types with a semantic contract. It is used via inheritance. The user
///        has to also define the maximum capacity, a callable which defines
///        invalid characters as well as a callable which defines invalid content.
/// @code
/// bool user_name_does_contain_invalid_characters(const string<platform::MAX_USER_NAME_LENGTH>& value) noexcept;
/// bool user_name_does_contain_invalid_content(const string<platform::MAX_USER_NAME_LENGTH>& value) noexcept;
///
/// // define custom semantic string UserName
/// class UserName : public SemanticString<platform::MAX_USER_NAME_LENGTH,
///                                        user_name_does_contain_invalid_content,
///                                        user_name_does_contain_invalid_characters>
/// {
/// };
/// @endcode
/// @note Since the inner logic of the SemanticString is always the same additional
///         implementations can be verified directly by the test suite defined in
///         'test_vocabulary_semantic_string.cpp'.
///         One has to only add the specific implementation to the 'Implementations'
///         type list.
template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
class SemanticString
{
  public:
    /// @brief Creates a new SemanticString from the provided string literal.
    ///         If the value contains invalid characters or invalid content
    ///         the expected returns an error describing the cause.
    /// @param[in] value the value of the SemanticString
    /// @return expected either containing the new SemanticString or an error
    template <uint64_t N>
    // avoid-c-arrays: we would like to assign string_literals, safe since it is known
    //                 at compile time.
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)
    static expected<SemanticString, SemanticStringError> create(const char (&value)[N]) noexcept;

    /// @brief Creates a new SemanticString from the provided string.
    ///         If the value contains invalid characters or invalid content
    ///         the expected returns an error describing the cause.
    /// @param[in] value the value of the SemanticString
    /// @return expected either containing the new SemanticString or an error
    template <uint64_t N>
    static expected<SemanticString, SemanticStringError> create(const string<N>& value) noexcept;

    /// @brief Returns the number of characters.
    /// @return number of characters
    constexpr uint64_t size() const noexcept;

    /// @brief Returns the capacity of the string.
    /// @return the maximum amount of characters which can be stored.
    static constexpr uint64_t capacity() noexcept;

    /// @brief Returns a const reference to the underlying string. It is const
    ///         and shall not be modified to guarantee the contract that a
    ///         SemanticString contains always a valid value.
    /// @return string reference containing the actual value.
    constexpr const string<Capacity>& as_string() const noexcept;

    /// @brief Appends another string to the SemanticString. If the value contains
    ///        invalid characters or the result would end up in invalid content
    ///        it fails.
    /// @param[in] value the value which should be added
    /// @return on failure the error inside the expected describes the failure
    template <typename T>
    expected<SemanticStringError> append(const T& value) noexcept;

    /// @brief Inserts another string into the SemanticString. If the value contains
    ///        invalid characters or the result would end up in invalid content
    ///        it fails.
    /// @param[in] pos the position where the string should be inserted
    /// @param[in] str the value which should be added
    /// @param[in] count how many characters of str shall be inserted
    /// @return on failure the error inside the expected describes the failure
    template <typename T>
    expected<SemanticStringError> insert(const uint64_t pos, const T& str, const uint64_t count) noexcept;

  private:
    template <uint64_t N>
    explicit SemanticString(const string<N>& value) noexcept;

    template <uint64_t N>
    static expected<SemanticString, SemanticStringError> create_impl(const char* value) noexcept;

  private:
    string<Capacity> m_data;
};
} // namespace iox

#include "iox/detail/semantic_string.inl"

#endif
