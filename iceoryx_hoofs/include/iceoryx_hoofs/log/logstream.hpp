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
#include "iceoryx_hoofs/log/logger.hpp"

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
    explicit constexpr LogHex(const T value) noexcept;

  private:
    T m_value;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
constexpr LogHex<T> hex(const T value) noexcept;

LogHex<uint64_t> hex(const void* const ptr) noexcept;

template <typename T>
class LogOct
{
  public:
    friend class LogStream;

    template <typename = std::enable_if_t<std::is_integral<T>::value>>
    inline explicit constexpr LogOct(const T value) noexcept;

  private:
    T m_value;
};

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
inline constexpr LogOct<T> oct(const T value) noexcept;

/// @todo iox-#1345 implement LogBin and LogRawBuffer

class LogStream
{
  public:
    LogStream(Logger& logger, const char* file, const int line, const char* function, LogLevel logLevel) noexcept;

    LogStream(const char* file, const int line, const char* function, LogLevel logLevel) noexcept;

    virtual ~LogStream() noexcept;

    LogStream(const LogStream&) = delete;
    LogStream(LogStream&&) = delete;

    LogStream& operator=(const LogStream&) = delete;
    LogStream& operator=(LogStream&&) = delete;

    LogStream& self() noexcept;

    LogStream& operator<<(const char* cstr) noexcept;

    /// @todo iox-#1345 instead of using std::string we could also accept everything with a c_str() method
    /// and avoid the std::string dependency
    LogStream& operator<<(const std::string& str) noexcept;

    LogStream& operator<<(const bool val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_arithmetic<T>::value, int> = 0>
    LogStream& operator<<(const T val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
    LogStream& operator<<(const LogHex<T> val) noexcept;

    template <typename T, typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
    LogStream& operator<<(const LogOct<T> val) noexcept;

    /// @code
    /// IOX_LOG(INFO) << "#### Hello " << [] (auto& stream) -> auto& { stream << "World"; return stream; };
    /// @endcode
    template <typename Callable,
              typename = std::enable_if_t<cxx::is_invocable_r<LogStream&, Callable, LogStream&>::value>>
    LogStream& operator<<(const Callable&& c);

    LogStream& operator<<(const LogLevel value) noexcept;

  private:
    void flush() noexcept;

    // JUSTIFICATION it is fine to use a reference since the LogStream object is intentionally not movable
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    Logger& m_logger;
    bool m_isFlushed{false};
};

} // namespace log
} // namespace iox

#include "iceoryx_hoofs/internal/log/logstream.inl"

#endif // IOX_HOOFS_LOG_LOGSTREAM_HPP
