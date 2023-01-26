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

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iox/string.hpp"

#include <cstring>
#include <iostream>
#include <type_traits>

#include "iceoryx_platform/platform_correction.hpp"
#include "iceoryx_platform/platform_settings.hpp"

namespace iox
{
template <uint64_t Capacity>
class string;
struct TruncateToCapacity_t;
namespace cxx
{
namespace internal
{
// AXIVION DISABLE STYLE AutosarC++19_03-A3.9.1: Not used as an integer but as actual character.
constexpr char ASCII_A{'a'};
constexpr char ASCII_Z{'z'};
constexpr char ASCII_CAPITAL_A{'A'};
constexpr char ASCII_CAPITAL_Z{'Z'};
constexpr char ASCII_0{'0'};
constexpr char ASCII_9{'9'};
constexpr char ASCII_MINUS{'-'};
constexpr char ASCII_DOT{'.'};
constexpr char ASCII_COLON{':'};
constexpr char ASCII_UNDERSCORE{'_'};
} // namespace internal
// AXIVION ENABLE STYLE AutosarC++19_03-A3.9.1

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

    // AXIVION Next Construct AutosarC++19_03-A13.5.2,AutosarC++19_03-A13.5.3:this should behave like a pointer which never can be nullptr,
    // adding explicit would defeat the purpose
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator T() const noexcept
    {
        return m_value;
    }

  private:
    T m_value;
};

/// @brief Get the capacity of a C array at compile time
/// @code
/// constexpr uint32_t FOO[42]{};
/// IOX_LOG(INFO) << arrayCapacity(FOO); // will print 42
/// @endcode
/// @tparam T the type of the array filled out by the compiler.
/// @tparam CapacityValue the capacity of the array filled out by the compiler.
/// @param[in] The actual content of the array is not of interest. Its just the capacity of the array that matters.
/// @return Returns the capacity of the array at compile time.
template <typename T, uint64_t CapacityValue>
// AXIVION Next Construct AutosarC++19_03-A18.1.1:returning capacity of C array at compile time is safe, no
// possibility of out of bounds access
// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
static constexpr uint64_t arrayCapacity(T const (&/*notInterested*/)[CapacityValue]) noexcept
{
    return CapacityValue;
}

/// @brief Returns info whether called on a 32-bit system
/// @return True if called on 32-bit, false if not 32-bit system
constexpr bool isCompiledOn32BitSystem() noexcept
{
    return INTPTR_MAX == INT32_MAX;
}

enum class RelativePathComponents : std::uint32_t
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
bool isValidPathEntry(const string<StringCapacity>& name, const RelativePathComponents relativePathComponents) noexcept;

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
constexpr DestinationType from(const SourceType value);

// Using a struct as impl, as free functions do not support partially specialized templates
template <typename SourceType, typename DestinationType>
struct FromImpl
{
    static DestinationType fromImpl(const SourceType& value);
};

/// @brief Converts a value of type SourceType to a corresponding value of type DestinationType. This is a convenience
/// function which is automatically available when `from` is implemented. This function shall therefore not be
/// specialized but always the `from` function.
/// @code
/// Bar b = iox::cxx::into<Bar>(Foo::ENUM_VALUE);
/// @endcode
/// @tparam DestinationType is the 'to' type
/// @tparam SourceType is the 'from' type
/// @param[in] value of type SourceType to convert to DestinationType
/// @return converted value of SourceType to corresponding value of DestinationType
template <typename DestinationType, typename SourceType>
constexpr DestinationType into(const SourceType value);
} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/helplets.inl"

#endif // IOX_HOOFS_CXX_HELPLETS_HPP
