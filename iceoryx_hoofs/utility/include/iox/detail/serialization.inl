// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_UTILITY_SERIALIZATION_INL
#define IOX_HOOFS_UTILITY_SERIALIZATION_INL

#include "iox/detail/serialization.hpp"

namespace iox
{
inline Serialization::operator std::string() const noexcept
{
    return m_value;
}

inline std::string Serialization::toString() const noexcept
{
    return m_value;
}

template <typename... Targs>
inline Serialization Serialization::create(const Targs&... args) noexcept
{
    return Serialization(serializer(args...));
}

template <typename T, typename... Targs>
inline bool Serialization::extract(T& t, Targs&... args) const noexcept
{
    return deserialize(m_value, t, args...);
}

inline Serialization::Serialization(const std::string& f_value) noexcept
    : m_value(f_value)
{
}

inline std::string Serialization::serializer() noexcept
{
    return std::string();
}

template <typename T>
inline typename std::enable_if<!std::is_convertible<T, Serialization>::value, std::string>::type
Serialization::getString(const T& t) noexcept
{
    return convert::toString(t);
}

template <typename T>
inline typename std::enable_if<std::is_convertible<T, Serialization>::value, std::string>::type
Serialization::getString(const T& t) noexcept
{
    const auto serial = static_cast<Serialization>(t);
    return serial.toString();
}

template <typename T, typename... Targs>
inline std::string Serialization::serializer(const T& t, const Targs&... args) noexcept
{
    std::string serializedString = getString(t);
    std::string serializedStringLength = convert::toString(serializedString.size());

    return serializedStringLength + SEPARATOR + serializedString + serializer(args...);
}

inline bool Serialization::deserialize(const std::string& serializedString) noexcept
{
    return serializedString.empty();
}

template <typename T, typename... Targs>
inline bool Serialization::deserialize(const std::string& serializedString, T& t, Targs&... args) noexcept
{
    std::string remainder = serializedString;
    std::string entry;

    if (!removeFirstEntry(entry, remainder))
    {
        return false;
    }

    auto result = convert::from_string<T>(entry.c_str());

    if (!result.has_value())
    {
        return false;
    }

    t = result.value();
    return deserialize(remainder, args...);
}

inline bool Serialization::removeFirstEntry(std::string& firstEntry, std::string& remainder) noexcept
{
    auto pos = remainder.find_first_of(SEPARATOR);
    if (pos == std::string::npos)
    {
        return false;
    }

    auto result = convert::from_string<decltype(pos)>(remainder.substr(0U, pos).c_str());

    if (!result.has_value())
    {
        return false;
    }

    const auto length = result.value();

    if (remainder.size() < pos + length + 1U)
    {
        return false;
    }

    firstEntry = remainder.substr(pos + 1U, length);
    remainder = remainder.substr(pos + 1U + length);

    return true;
}

template <typename T>
inline bool Serialization::getNth(const unsigned int index, T& t) const noexcept
{
    std::string entry;
    std::string remainder = m_value;
    for (unsigned int i = 0; i < index + 1; ++i)
    {
        if (!removeFirstEntry(entry, remainder))
        {
            return false;
        }
    }
    return convert::from_string<T>(entry.c_str()).and_then([&t](const auto& value) { t = value; }).has_value();
}
} // namespace iox

#endif // IOX_HOOFS_UTILITY_SERIALIZATION_INL
