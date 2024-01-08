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

#ifndef IOX_HOOFS_REPORTING_ERROR_REPORTING_MACROS_HPP
#define IOX_HOOFS_REPORTING_ERROR_REPORTING_MACROS_HPP

#include "iox/error_reporting/configuration.hpp"
#include "iox/error_reporting/error_forwarding.hpp"
#include "iox/error_reporting/types.hpp"

#include "iox/error_reporting/source_location.hpp"

// ***
// * Define public error reporting API
// ***

// NOLINTBEGIN(cppcoreguidelines-macro-usage) source location requires macros

// The following macros are statements (not expressions).
// This is important, as it enforces correct use to some degree.
// For example they cannot be used as function arguments and must be terminated with a ';'.
//
// Note: once source location becomes available without macro usage this could (and arguably should)
// be transformed into a function API.

/// @brief report error of some non-fatal kind
/// @param error error object (or code)
/// @param kind kind of error, must be non-fatal
#define IOX_REPORT(error, kind)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        iox::er::forwardNonFatalError(iox::er::toError(error), kind, CURRENT_SOURCE_LOCATION);                         \
    } while (false)

/// @brief report fatal error
/// @param error error object (or code)
#define IOX_REPORT_FATAL(error)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        iox::er::forwardFatalError(iox::er::toError(error), FATAL, CURRENT_SOURCE_LOCATION);                           \
    } while (false)

/// @brief report error of some non-fatal kind if expr evaluates to true
/// @param expr boolean expression
/// @param error error object (or code)
/// @param kind kind of error, must be non-fatal
#define IOX_REPORT_IF(expr, error, kind)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (expr)                                                                                                      \
        {                                                                                                              \
            iox::er::forwardNonFatalError(                                                                             \
                iox::er::toError(error),                                                                               \
                kind,                                                                                                  \
                CURRENT_SOURCE_LOCATION); /* @todo iox-#1032 add strigified 'expr' as '#expr' */                       \
        }                                                                                                              \
    } while (false)

/// @brief report fatal error if expr evaluates to true
/// @param expr boolean expression
/// @param error error object (or code)
#define IOX_REPORT_FATAL_IF(expr, error)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (expr)                                                                                                      \
        {                                                                                                              \
            iox::er::forwardFatalError(iox::er::toError(error), FATAL, CURRENT_SOURCE_LOCATION);                       \
        }                                                                                                              \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif // IOX_HOOFS_REPORTING_ERROR_REPORTING_MACROS_HPP
