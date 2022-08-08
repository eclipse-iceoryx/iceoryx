// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_LOG_LOGSTREAM_HPP
#define IOX_HOOFS_LOG_LOGSTREAM_HPP

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/log_ng/logger.hpp"

#include <string>

namespace iox
{
namespace log
{
class LogStream;

template <typename T>
class LogHex
{
  public:
    friend class LogStream;

    template <typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    inline explicit constexpr LogHex(const T value) noexcept;

  private:
    T m_value;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
inline constexpr LogHex<T> hex(const T value) noexcept;

inline LogHex<uint64_t> hex(const void* const ptr) noexcept;

template <typename T>
class LogOct
{
  public:
    friend class LogStream;

    template <typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    inline explicit constexpr LogOct(const T value) noexcept;

  private:
    T m_value;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
inline constexpr LogOct<T> oct(const T value) noexcept;

class LogStream
{
  public:
    inline LogStream(const char* file, const int line, const char* function, LogLevel logLevel) noexcept;

    inline virtual ~LogStream() noexcept;

    LogStream(const LogStream&) = delete;
    LogStream(LogStream&&) = delete;

    LogStream& operator=(const LogStream&) = delete;
    LogStream& operator=(LogStream&&) = delete;

    inline void flush() noexcept;

    inline LogStream& self() noexcept;

    inline LogStream& operator<<(const char* cstr) noexcept;

    // TODO instead of using std::string we could also accecpt everything with a c_str() method and avoid the
    // std::string dependency
    inline LogStream& operator<<(const std::string& str) noexcept;

    template <typename T, typename std::enable_if_t<std::is_signed<T>::value, int> = 0>
    inline LogStream& operator<<(const T val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    inline LogStream& operator<<(const T val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_signed<T>::value, int> = 0>
    inline LogStream& operator<<(const LogHex<T>&& val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    inline LogStream& operator<<(const LogHex<T>&& val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_signed<T>::value, int> = 0>
    inline LogStream& operator<<(const LogOct<T>&& val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_unsigned<T>::value, int> = 0>
    inline LogStream& operator<<(const LogOct<T>&& val) noexcept;

    /// @code
    /// IOX_LOG(INFO) << "#### Hello " << [] (auto& stream) -> auto& { stream << "World"; return stream; };
    /// @endcode
    template <typename Callable,
              typename = std::enable_if_t<cxx::is_invocable_r<LogStream&, Callable, LogStream&>::value>>
    inline LogStream& operator<<(const Callable&& c);

    inline LogStream& operator<<(const LogLevel value) noexcept;

  private:
    Logger& m_logger{Logger::get()};
    bool m_flushed{false};
};

} // namespace log
} // namespace iox

#include "iceoryx_hoofs/internal/log_ng/logstream.inl"

#endif // IOX_HOOFS_LOG_LOGSTREAM_HPP
