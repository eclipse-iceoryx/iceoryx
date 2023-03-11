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

#ifndef IOX_HOOFS_ERROR_REPORTING_API_HPP
#define IOX_HOOFS_ERROR_REPORTING_API_HPP

#include "iceoryx_hoofs/error_reporting/configuration.hpp"
#include "iceoryx_hoofs/error_reporting/custom/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_forwarding.hpp"

#include "iceoryx_hoofs/error_reporting/auxiliary_macros.hpp"
#include "iceoryx_hoofs/error_reporting/source_location.hpp"

// ***
// * Define public error reporting API
// ***

// NOLINTBEGIN(cppcoreguidelines-macro-usage) source location requires macros

// The following macros are statements (not expressions).
// This is important, as it enforces correct use to some degree.
// For example thye cannot be used as function arguments and must be terminated with a ';'.
//
// Note: once source location becomes available without macro usage this could (and arguably should)
// be transformed into a function API.

/// @brief calls panic handler and does not return
/// @param msg optional message string literal
/// @note could actually throw if desired without breaking control flow asssumptions
#define IOX_PANIC(...)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        iox::err::panic(CURRENT_SOURCE_LOCATION, ##__VA_ARGS__);                                                       \
    } while (false)

/// @brief report error of some kind
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT(error, kind)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (isFatal(kind))                                                                                             \
        {                                                                                                              \
            forwardFatalError(iox::err::toError(error), kind, CURRENT_SOURCE_LOCATION);                                \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            forwardNonFatalError(iox::err::toError(error), kind, CURRENT_SOURCE_LOCATION);                             \
        }                                                                                                              \
    } while (false)

/// @brief report fatal error
/// @param error error object (or code)
#define IOX_REPORT_FATAL(error) IOX_REPORT(error, FATAL)

/// @brief report error of some kind if expr evaluates to true
/// @param expr boolean expression
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT_IF(expr, error, kind)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (expr)                                                                                                      \
        {                                                                                                              \
            if (isFatal(kind))                                                                                         \
            {                                                                                                          \
                forwardFatalError(iox::err::toError(error), kind, CURRENT_SOURCE_LOCATION);                            \
            }                                                                                                          \
            else                                                                                                       \
            {                                                                                                          \
                forwardNonFatalError(iox::err::toError(error), kind, CURRENT_SOURCE_LOCATION);                         \
            }                                                                                                          \
        }                                                                                                              \
    } while (false)

/// @brief report fatal error if expr evaluates to false
/// @note for conditions that may actually happen during correct use
/// @param expr boolean expression that must hold
/// @param error error object (or code)
#define IOX_REQUIRE(expr, error) IOX_REPORT_IF(!(expr), error, FATAL)

//************************************************************************************************
//* For documentation of intent, defensive programming and debugging
//*
//* There are no error codes/errors required here on purpose, as it would make the use cumbersome.
//* Instead a special internal error type is used.
//************************************************************************************************

/// @brief if enabled: report fatal precondition violation if expr evaluates to false
/// @param arg1 boolean expression that must hold upon entry of the function it appears in
/// @param arg2 optional message to be logged in case of violation
#define IOX_PRECONDITION(...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (iox::err::Configuration::CHECK_PRECONDITIONS && !(FIRST_ARG_(__VA_ARGS__)))                                \
        {                                                                                                              \
            iox::err::forwardFatalError(iox::err::Violation::createPreconditionViolation(),                            \
                                        iox::err::PRECONDITION_VIOLATION,                                              \
                                        CURRENT_SOURCE_LOCATION,                                                       \
                                        SECOND_ARG_OR_EMPTY_(__VA_ARGS__));                                            \
        }                                                                                                              \
    } while (false)

/// @brief if enabled: report (fatal) assumption violation if expr evaluates to false
/// @note for conditions that should not happen with correct use
/// @param arg1 boolean expression that must hold
/// @param arg2 optional message to be logged in case of violation
#define IOX_ASSUME(...)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        if (iox::err::Configuration::CHECK_ASSUMPTIONS && !(FIRST_ARG_(__VA_ARGS__)))                                  \
        {                                                                                                              \
            iox::err::forwardFatalError(iox::err::Violation::createAssumptionViolation(),                              \
                                        iox::err::ASSUMPTION_VIOLATION,                                                \
                                        CURRENT_SOURCE_LOCATION,                                                       \
                                        SECOND_ARG_OR_EMPTY_(__VA_ARGS__));                                            \
        }                                                                                                              \
    } while (false)
/// @brief panic if control flow reaches this code at runtime
#define IOX_UNREACHABLE()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        iox::err::panic(CURRENT_SOURCE_LOCATION, "Reached code that was supposed to be unreachable.");                 \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
