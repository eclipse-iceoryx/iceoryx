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
    Serialization serial = static_cast<Serialization>(t);
    return serial.toString();
}

template <typename T, typename... Targs>
inline std::string Serialization::serializer(const T& t, const Targs&... args) noexcept
{
    std::string serializedString = getString(t);
    std::string serializedStringLength = convert::toString(serializedString.size());

    return serializedStringLength + separator + serializedString + serializer(args...);
}

inline bool Serialization::deserialize(const std::string& serializedString) noexcept
{
    if (serializedString.empty())
    {
        return true;
    }
    else
    {
        return false;
    }
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

    if (!convert::fromString(entry.c_str(), t))
    {
        return false;
    }

    return deserialize(remainder, args...);
}

inline bool Serialization::removeFirstEntry(std::string& firstEntry, std::string& remainder) noexcept
{
    size_t pos = remainder.find_first_of(separator);
    if (pos == std::string::npos)
    {
        return false;
    }

    size_t length;
    if (!convert::fromString(remainder.substr(0, pos).c_str(), length))
    {
        return false;
    }

    if (remainder.size() < pos + length + 1)
    {
        return false;
    }

    firstEntry = remainder.substr(pos + 1, length);
    remainder = remainder.substr(pos + 1 + length);

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

    if (!convert::fromString(entry.c_str(), t))
    {
        return false;
    }

    return true;
}
} // namespace cxx
} // namespace iox
