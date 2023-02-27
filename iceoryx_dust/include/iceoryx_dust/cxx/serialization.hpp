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
#ifndef IOX_DUST_CXX_SERIALIZATION_HPP
#define IOX_DUST_CXX_SERIALIZATION_HPP

#include "iceoryx_dust/cxx/convert.hpp"
#include "iceoryx_dust/cxx/std_string_support.hpp"

#include <iostream>
#include <sstream>

namespace iox
{
namespace cxx
{
/// @brief Simple serializer which serials every given type into the following
///         format: (The type needs to be convertable into a string via cxx::convert::toString)
///             LENGTH:DATALENGTH:DATA...
///         Example: Serializes "hello", 123, 123.01 into
///             5:hello3:1236:123.01
/// @code
///     auto serial = cxx::Serialization::create("fuu", 123, 12.12f, 'c');
///     IOX_LOG(INFO) << serial.toString();
///
///     std::string v1;
///     int v2;
///     float v3;
///     char v4;
///
///     if ( serial.extract(v1, v2, v3, v4) ) {} // succeeds since every value is convertable
///
///     if ( serial.getNth(0, v2) ) {} // fails since "fuu" is not an integer
///
///     // if you'd like to write a serializable class they need to have a CTor
///     // with a cxx::Serialization argument and an operator cxx::Serialization
///     class Fuu {
///         public:
///             Fuu(const cxx::Serialization & s) {
///                 if ( !s.Extract(v1, v2, v3) ) {} // error handling
///             }
///             operator cxx::Serialization() const {
///                 return cxx::Serialization::Create(v1, v2, v3);
///             }
///         private:
///             int v1 = 123;
///             char v2 = 'c';
///             std::string v3 = "hello world";
///
///     };
/// @endcode
class Serialization
{
  public:
    /// @brief Creates a serialization object from a given raw serialization
    /// @param[in] value string of serialized data
    explicit Serialization(const std::string& value) noexcept;

    /// @brief string conversion operator, returns the raw serialized string
    /// @return serialized string
    std::string toString() const noexcept;

    /// @brief string conversion operator, returns the raw serialized string
    /// @return serialized string
    // the whole file will be refactored to remove 'std::string'; this lint will also be handled with at the refactoring
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    operator std::string() const noexcept;

    /// @brief Create Serialization if every arguments is convertable to string
    ///         via cxx::convert::toString, this means if the argument is either
    ///         a pod (plain old data) type or is convertable to string (operator std::string())
    /// @param[in] args list of string convertable data
    /// @return Serialization object which contains the serialized data
    template <typename... Targs>
    static Serialization create(const Targs&... args) noexcept;

    /// @brief Extracts the values from the serialization and writes them into the
    ///         the given args, if one value is not
    ///         convertable it returns false (e.g. convert "hello" to an integer)
    ///         It also returns false if the underlying serialization string has a
    ///         wrong syntax
    /// @param[in] t reference where the first value in the serialization will be stored in
    /// @param[in] args reference where the remainding values in the serialization will be stored in
    /// @return true if extraction of all values was successfull, otherwise false
    template <typename T, typename... Targs>
    bool extract(T& t, Targs&... args) const noexcept;

    /// @brief Extracts the value at index and writes it into t. If the conversion
    ///         failed it returns false
    ///         It also returns false if the underlying serialization string has a
    ///         wrong syntax
    /// @param[in] index index to the value which should be extracted
    /// @param[in] t variable where the value should be stored
    /// @return true if extraction was successful, otherwise false
    template <typename T>
    bool getNth(const unsigned int index, T& t) const noexcept;

    /// @brief This is an error which can be used for 'iox::expected' on a custom deserialization when 'extract' fails
    enum class Error
    {
        DESERIALIZATION_FAILED, ///< indicates a failed deserialization
    };

  private:
    std::string m_value;
    static constexpr char SEPARATOR = ':';

  private:
    static std::string serializer() noexcept;

    static bool removeFirstEntry(std::string& firstEntry, std::string& remainder) noexcept;

    template <typename T>
    static typename std::enable_if<std::is_convertible<T, Serialization>::value, std::string>::type
    getString(const T& t) noexcept;
    template <typename T>
    static typename std::enable_if<!std::is_convertible<T, Serialization>::value, std::string>::type
    getString(const T& t) noexcept;
    template <typename T, typename... Targs>
    static std::string serializer(const T& t, const Targs&... args) noexcept;

    static bool deserialize(const std::string& serializedString) noexcept;

    template <typename T, typename... Targs>
    static bool deserialize(const std::string& serializedString, T& t, Targs&... args) noexcept;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_dust/internal/cxx/serialization.inl"

#endif // IOX_DUST_CXX_SERIALIZATION_HPP
