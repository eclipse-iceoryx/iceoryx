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

// TODO: doc add tests to the semantic string test suite
template <uint64_t Capacity,
          DoesContainInvalidContent<Capacity> DoesContainInvalidContentCall,
          DoesContainInvalidCharacter<Capacity> DoesContainInvalidCharacterCall>
class SemanticString
{
  public:
    template <uint64_t N>
    // avoid-c-arrays: we would like to assign string_literals, safe since it is known
    //                 at compile time.
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)
    static expected<SemanticString, SemanticStringError> create(const char (&value)[N]) noexcept;

    template <uint64_t N>
    static expected<SemanticString, SemanticStringError> create(const string<N>& value) noexcept;

    constexpr uint64_t size() const noexcept;

    static constexpr uint64_t capacity() noexcept;

    constexpr const string<Capacity>& as_string() const noexcept;

    template <typename T>
    expected<SemanticStringError> append(const T& value) noexcept;

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
