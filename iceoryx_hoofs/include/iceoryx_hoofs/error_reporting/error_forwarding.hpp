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

#include <utility>

namespace iox
{
namespace err
{
/// @brief Forwards a fatal error and does not return.
/// @param location the location of the error
/// @param error the error
/// @param kind the kind of error (category)
template <typename Error, typename Kind>
[[noreturn]] inline void forwardFatalError(Error&& error, Kind&& kind, const SourceLocation& location)
{
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
    report(location, std::forward<Kind>(kind), std::forward<Error>(error), std::forward<Message>(msg));
    panic(location);
    abort();
}

} // namespace err
} // namespace iox

#endif
