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

#ifndef IOX_HOOFS_ERROR_REPORTING_ERROR_FORWARDING_HPP
#define IOX_HOOFS_ERROR_REPORTING_ERROR_FORWARDING_HPP

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/source_location.hpp"

// to establish connection to the custom implementation
#include "iceoryx_hoofs/error_reporting/custom/error_reporting.hpp"

#include <type_traits>
#include <utility>

namespace iox
{
namespace er
{
// This is lightweight and only exists to hide some complexity that would otherwise be part of the
// macro API.

/// @brief Forwards that a panic state was encountered and does not return.
/// @param location the location of the panic invocation
/// @param msg the message to be forwarded
/// @note required to enforce no return
template <typename Message>
[[noreturn]] inline void forwardPanic(const SourceLocation& location, Message&& msg)
{
    panic(location, std::forward<Message>(msg));
    abort();
}

/// @brief Forwards a fatal error and does not return.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
template <typename Error, typename Kind>
[[noreturn]] inline void forwardFatalError(Error&& error, Kind&& kind, const SourceLocation& location)
{
    using K = typename std::remove_const<typename std::remove_reference<Kind>::type>::type;
    static_assert(IsFatal<K>::value, "Must forward a fatal error!");

    report(location, std::forward<Kind>(kind), std::forward<Error>(error));
    panic(location);
    abort();
}

/// @brief Forwards a non-fatal error.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
template <typename Error, typename Kind>
inline void forwardNonFatalError(Error&& error, Kind&& kind, const SourceLocation& location)
{
    using K = typename std::remove_const<typename std::remove_reference<Kind>::type>::type;
    static_assert(!IsFatal<K>::value, "Must forward a non-fatal error!");

    report(location, std::forward<Kind>(kind), std::forward<Error>(error));
}

/// @brief Forwards a fatal error and a message and does not return.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
/// @param msg the message to be forwarded
template <typename Error, typename Kind, typename Message>
[[noreturn]] inline void forwardFatalError(Error&& error, Kind&& kind, const SourceLocation& location, Message&& msg)
{
    using K = typename std::remove_const<typename std::remove_reference<Kind>::type>::type;
    static_assert(IsFatal<K>::value, "Must forward a fatal error!");

    report(location, std::forward<Kind>(kind), std::forward<Error>(error), std::forward<Message>(msg));
    panic(location);
    abort();
}

} // namespace er
} // namespace iox

#endif
