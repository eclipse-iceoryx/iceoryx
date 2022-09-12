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
#ifndef IOX_HOOFS_CXX_HELPLETS_HPP
#define IOX_HOOFS_CXX_HELPLETS_HPP

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <type_traits>

#include "iceoryx_platform/platform_correction.hpp"
#include "iceoryx_platform/platform_settings.hpp"

namespace iox
{
namespace cxx
{
template <uint64_t Capacity>
class string;
struct TruncateToCapacity_t;

namespace internal
{
/// @brief struct to find the best fitting unsigned integer type
template <bool GreaterUint8, bool GreaterUint16, bool GreaterUint32>
struct BestFittingTypeImpl
{
    using Type_t = uint64_t;
};

template <>
struct BestFittingTypeImpl<false, false, false>
{
    using Type_t = uint8_t;
};

template <>
struct BestFittingTypeImpl<true, false, false>
{
    using Type_t = uint16_t;
};

template <>
struct BestFittingTypeImpl<true, true, false>
{
    using Type_t = uint32_t;
};

constexpr char ASCII_A = 'a';
constexpr char ASCII_Z = 'z';
constexpr char ASCII_CAPITAL_A = 'A';
constexpr char ASCII_CAPITAL_Z = 'Z';
constexpr char ASCII_0 = '0';
constexpr char ASCII_9 = '9';
constexpr char ASCII_MINUS = '-';
constexpr char ASCII_DOT = '.';
constexpr char ASCII_COLON = ':';
constexpr char ASCII_UNDERSCORE = '_';
} // namespace internal

template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value, void>::type>
struct not_null
{
  public:
    // this class should behave like a pointer which never can be nullptr, adding explicit
    // would defeat the purpose
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    not_null(T t) noexcept
        : m_value(t)
    {
        Expects(t != nullptr);
    }

    // this should behave like a pointer which never can be nullptr, adding explicit
    // would defeat the purpose
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator T() const noexcept
    {
        return m_value;
    }

  private:
    T m_value;
};

template <typename T, T Minimum>
struct greater_or_equal
{
  public:
    // this class should behave like a T but which never can be less than Minimum.
    // Adding explicit would defeat the purpose.
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    greater_or_equal(T t) noexcept
        : m_value(t)
    {
        Expects(t >= Minimum);
    }

    // this class should behave like a T but which never can be less than Minimum.
    // Adding explicit would defeat the purpose.
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator T() const noexcept
    {
        return m_value;
    }

  private:
    T m_value;
};

template <typename T, T Minimum, T Maximum>
struct range
{
  public:
    // this class should behave like a T but with values only in range [Minimum, Maximum]
    // Adding explicit would defeat the purpose.
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    range(T t) noexcept
        : m_value(t)
    {
        Expects(t >= Minimum && t <= Maximum);
    }

    // this class should behave like a T but with values only in range [Minimum, Maximum]
    // Adding explicit would defeat the purpose.
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator T() const noexcept
    {
        return m_value;
    }

  private:
    T m_value;
};

template <typename T>
T align(const T value, const T alignment) noexcept
{
    T remainder = value % alignment;
    return value + ((remainder == 0U) ? 0U : alignment - remainder);
}

/// @brief allocates aligned memory which can only be free'd by alignedFree
/// @param[in] alignment, alignment of the memory
/// @param[in] size, memory size
/// @return void pointer to the aligned memory
void* alignedAlloc(const uint64_t alignment, const uint64_t size) noexcept;

/// @brief frees aligned memory allocated with alignedAlloc
/// @param[in] memory, pointer to the aligned memory
void alignedFree(void* const memory) noexcept;

/// template recursion stopper for maximum alignment calculation
template <size_t S = 0>
constexpr size_t maxAlignment() noexcept
{
    return S;
}

/// calculate maximum alignment of supplied types
template <typename T, typename... Args>
constexpr size_t maxAlignment() noexcept
{
    return alignof(T) > maxAlignment<Args...>() ? alignof(T) : maxAlignment<Args...>();
}

/// template recursion stopper for maximum size calculation
template <size_t S = 0>
constexpr size_t maxSize() noexcept
{
    return S;
}

/// calculate maximum size of supplied types
template <typename T, typename... Args>
constexpr size_t maxSize() noexcept
{
    return sizeof(T) > maxSize<Args...>() ? sizeof(T) : maxSize<Args...>();
}

/// @brief Get the capacity of a C array at compile time
/// @code
/// constexpr uint32_t FOO[42]{};
/// std::cout << arrayCapacity(FOO) << std::endl; // will print 42
/// @endcode
/// @tparam T the type of the array filled out by the compiler.
/// @tparam CapacityValue the capacity of the array filled out by the compiler.
/// @param[in] The actual content of the array is not of interest. Its just the capacity of the array that matters.
/// @return Returns the capacity of the array at compile time.
template <typename T, uint64_t CapacityValue>
// returning capacity of C array at compile time is safe, no possibility of out of bounds access
// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
static constexpr uint64_t arrayCapacity(T const (&/*notInterested*/)[CapacityValue]) noexcept
{
    return CapacityValue;
}

/// @brief get the best fitting unsigned integer type for a given value at compile time
template <uint64_t Value>
struct BestFittingType
{
/// ignore the warnings because we need the comparisons to find the best fitting type
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    using Type_t = typename internal::BestFittingTypeImpl<(Value > std::numeric_limits<uint8_t>::max()),
                                                          (Value > std::numeric_limits<uint16_t>::max()),
                                                          (Value > std::numeric_limits<uint32_t>::max())>::Type_t;
#pragma GCC diagnostic pop
};

template <uint64_t Value>
using BestFittingType_t = typename BestFittingType<Value>::Type_t;

/// @brief Returns info whether called on a 32-bit system
/// @return True if called on 32-bit, false if not 32-bit system
constexpr bool isCompiledOn32BitSystem() noexcept
{
    return INTPTR_MAX == INT32_MAX;
}

/// @brief Checks if an unsigned integer is a power of two
/// @return true if power of two, otherwise false
template <typename T>
constexpr bool isPowerOfTwo(const T n) noexcept
{
    static_assert(std::is_unsigned<T>::value && !std::is_same<T, bool>::value, "Only unsigned integer are allowed!");
    return n && ((n & (n - 1U)) == 0U);
}

enum class RelativePathComponents
{
    REJECT,
    ACCEPT
};

/// @brief checks if the given string is a valid path entry. A path entry is the string between
///        two path separators.
/// @note A valid path entry for iceoryx must be platform independent and also supported
///       by various file systems. The file systems we intend to support are
///         * linux: ext3, ext4, btrfs
///         * windows: ntfs, exfat, fat
///         * freebsd: ufs, ffs
///         * apple: apfs
///         * qnx: etfs
///         * android: ext3, ext4, fat
///
///       Sometimes it is also possible that a certain file character is supported by the filesystem
///       itself but not by the platforms SDK. One example are files which end with a dot like "myFile."
///       which are supported by ntfs but not by the Windows SDK.
/// @param[in] name the path entry in question
/// @param[in] relativePathComponents are relative path components are allowed for this path entry
/// @return true if it is valid, otherwise false
template <uint64_t StringCapacity>
bool isValidPathEntry(const string<StringCapacity>& name,
                      const RelativePathComponents& relativePathComponents) noexcept;

/// @brief checks if the given string is a valid filename. It must fulfill the
///        requirements of a valid path entry (see, isValidPathEntry) and is not allowed
///        to contain relative path components
/// @param[in] name the string to verify
/// @return true if the string is a filename, otherwise false
template <uint64_t StringCapacity>
bool isValidFileName(const string<StringCapacity>& name) noexcept;

/// @brief verifies if the given string is a valid path to a file
/// @param[in] name the string to verify
/// @return true if the string is a path to a file, otherwise false
template <uint64_t StringCapacity>
bool isValidPathToFile(const string<StringCapacity>& name) noexcept;

/// @brief returns true if the provided name is a valid path, otherwise false
/// @param[in] name the string to verify
template <uint64_t StringCapacity>
bool isValidPathToDirectory(const string<StringCapacity>& name) noexcept;

/// @brief returns true if the provided name ends with a path separator, otherwise false
/// @param[in] name the string which may contain a path separator at the end
template <uint64_t StringCapacity>
bool doesEndWithPathSeparator(const string<StringCapacity>& name) noexcept;

/// @brief Converts a value of type F to a corresponding value of type T. This function needs to be specialized by the
/// user for the types to be converted.
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
/// @tparam F is the 'from' type
/// @tparam T is the 'to' type
/// @param[in] value of type F to convert to T
/// @return converted value of F to corresponding value of T
template <typename F, typename T>
constexpr T from(const F value) noexcept;

/// @brief Converts a value of type F to a corresponding value of type T. This is a convenience function which is
/// automatically available when `from` is implemented. This function shall therefore not be specialized but always the
/// `from` function.
/// @code
/// Bar b = iox::cxx::into<Bar>(Foo::ENUM_VALUE);
/// @endcode
/// @tparam T is the 'to' type
/// @tparam F is the 'from' type
/// @param[in] value of type F to convert to T
/// @return converted value of F to corresponding value of T
template <typename T, typename F>
constexpr T into(const F value) noexcept;
} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/helplets.inl"

#endif // IOX_HOOFS_CXX_HELPLETS_HPP
