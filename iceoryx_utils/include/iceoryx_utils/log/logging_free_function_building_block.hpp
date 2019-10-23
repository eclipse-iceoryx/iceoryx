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

#pragma once

/// @brief building block to easily create free function for logging in a library context
/// @code
/// // add this to e.g. foo_logging.hpp
/// // the logger can then be used with e.g. foo::LogInfo() of just LogInfo() in the same namespace
///
/// #ifndef FOO_LOGGING_HPP_INCLUDED
/// #define FOO_LOGGING_HPP_INCLUDED
///
/// #include "iceoryx_utils/log/logging_free_function_building_block.hpp"
///
/// namespace foo
/// {
///     struct LoggingComponent
///     {
///         static constexpr char Ctx[] = "FOO";
///         static constexpr char Description[] = "Log context of the FOO component!";
///     };
///
///     static constexpr auto LogFatal = iox::log::ffbb::LogFatal<LoggingComponent>;
///     static constexpr auto LogError = iox::log::ffbb::LogError<LoggingComponent>;
///     static constexpr auto LogWarn = iox::log::ffbb::LogWarn<LoggingComponent>;
///     static constexpr auto LogInfo = iox::log::ffbb::LogInfo<LoggingComponent>;
///     static constexpr auto LogDebug = iox::log::ffbb::LogDebug<LoggingComponent>;
///     static constexpr auto LogVerbose = iox::log::ffbb::LogVerbose<LoggingComponent>;
/// } // namespace foo
/// #endif // FOO_LOGGING_HPP_INCLUDED
///
///
/// // this needs to be in foo_logging.cpp
///
/// namespace foo
/// {
///     constexpr char ComponentPosh::Ctx[];
///     constexpr char ComponentPosh::Description[];
///
/// } // namespace foo
/// @endcode

#include "iceoryx_utils/log/logger.hpp"
#include "iceoryx_utils/log/logging.hpp"
#include "iceoryx_utils/log/logmanager.hpp"

namespace iox
{
namespace log
{
namespace ffbb
{
template <typename T>
static Logger& ComponentLogger()
{
    static auto& logger = CreateLogger(T::Ctx, T::Description, LogManager::GetLogManager().DefaultLogLevel());
    return logger;
}

template <typename T>
inline LogStream LogFatal() noexcept
{
    return ComponentLogger<T>().LogFatal();
}

template <typename T>
inline LogStream LogError() noexcept
{
    return ComponentLogger<T>().LogError();
}

template <typename T>
inline LogStream LogWarn() noexcept
{
    return ComponentLogger<T>().LogWarn();
}

template <typename T>
inline LogStream LogInfo() noexcept
{
    return ComponentLogger<T>().LogInfo();
}

template <typename T>
inline LogStream LogDebug() noexcept
{
    return ComponentLogger<T>().LogDebug();
}

template <typename T>
inline LogStream LogVerbose() noexcept
{
    return ComponentLogger<T>().LogVerbose();
}
} // namespace ffbb
} // namespace log
} // namespace iox

