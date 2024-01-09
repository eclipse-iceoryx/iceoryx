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

#ifndef IOX_HOOFS_REPORTING_ASSERTIONS_HPP
#define IOX_HOOFS_REPORTING_ASSERTIONS_HPP

#include "iox/error_reporting/configuration.hpp"
#include "iox/error_reporting/error_forwarding.hpp"

#include "iox/error_reporting/source_location.hpp"

// ***
// * Define public assertion API
// ***

// NOLINTBEGIN(cppcoreguidelines-macro-usage) source location requires macros

// The following macros are statements (not expressions).
// This is important, as it enforces correct use to some degree.
// For example they cannot be used as function arguments and must be terminated with a ';'.
//
// Note: once source location becomes available without macro usage this could (and arguably should)
// be transformed into a function API.

/// @brief calls panic handler and does not return
/// @param message message to be forwarded
/// @note could actually throw if desired without breaking control flow asssumptions
#define IOX_PANIC(message)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        iox::er::forwardPanic(CURRENT_SOURCE_LOCATION, message);                                                       \
    } while (false)

//************************************************************************************************
//* For documentation of intent, defensive programming and debugging
//*
//* There are no error codes/errors required here on purpose, as it would make the use cumbersome.
//* Instead a special internal error type is used.
//************************************************************************************************

/// @brief only for debug builds: report fatal assert violation if expr evaluates to false
/// @note for conditions that should not happen with correct use
/// @param expr boolean expression that must hold
/// @param message message to be forwarded in case of violation
#define IOX_ASSERT(expr, message)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if (iox::er::Configuration::CHECK_ASSERT && !(expr))                                                           \
        {                                                                                                              \
            iox::er::forwardFatalError(iox::er::Violation::createAssertViolation(),                                    \
                                       iox::er::ASSERT_VIOLATION,                                                      \
                                       CURRENT_SOURCE_LOCATION,                                                        \
                                       message);                                                                       \
        }                                                                                                              \
    } while (false)

/// @brief report fatal enforce violation if expr evaluates to false
/// @note for conditions that may actually happen during correct use
/// @param expr boolean expression that must hold
/// @param message message to be forwarded in case of violation
#define IOX_ENFORCE(expr, message)                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            iox::er::forwardFatalError(iox::er::Violation::createEnforceViolation(),                                   \
                                       iox::er::ENFORCE_VIOLATION,                                                     \
                                       CURRENT_SOURCE_LOCATION,                                                        \
                                       message); /* @todo iox-#1032 add strigified 'expr' as '#expr' */                \
        }                                                                                                              \
    } while (false)

/// @brief panic if control flow reaches this code at runtime
#define IOX_UNREACHABLE()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        iox::er::forwardPanic(CURRENT_SOURCE_LOCATION, "Reached code that was supposed to be unreachable.");           \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif // IOX_HOOFS_REPORTING_ASSERTIONS_HPP
