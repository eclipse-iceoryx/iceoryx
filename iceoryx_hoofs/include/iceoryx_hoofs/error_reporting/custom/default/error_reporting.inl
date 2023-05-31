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

#ifndef IOX_HOOFS_ERROR_REPORTING_CUSTOM_DEFAULT_ERROR_REPORTING_INL
#define IOX_HOOFS_ERROR_REPORTING_CUSTOM_DEFAULT_ERROR_REPORTING_INL

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/errors.hpp"
#include "iceoryx_hoofs/error_reporting/source_location.hpp"

#include "iceoryx_hoofs/error_reporting/custom/default/error_handler.hpp"
#include "iceoryx_hoofs/error_reporting/custom/error_kind.hpp"

namespace iox
{
namespace er
{

// The static reporting interface that must be defined to at least do nothing.
// It should provide a noreturn specification for panic (but since it be assumed that the custom
// code enforces this, it is enforced at the (non-custom) forwarding level.

// Here, the implementation redirects to the polymorphic handler interface.
// This adds an additional indirection but is required for switching handlers
// during operation.
// This is used for testing but must be done while no errors are reported concurrently,
// otherwise error notifations could be lost.

// The logging can be extended in the future.

// Custom panic
[[noreturn]] inline void panic()
{
    auto& h = ErrorHandler::get();
    h.onPanic();
    abort();
}

// Custom panic with location
[[noreturn]] inline void panic(const SourceLocation& location)
{
    IOX_ERROR_INTERNAL_LOG_PANIC(location) << "Panic";
    panic();
}

// Custom panic with location and message
// note that Message is generic as the logger technically accepts more general loggable constructs
// beyond const char*
template <class Message>
[[noreturn]] inline void panic(const SourceLocation& location, Message&& msg)
{
    IOX_ERROR_INTERNAL_LOG_PANIC(location) << "Panic " << msg;
    panic();
}

// Report any error, general version.
template <class Kind, class Error>
inline void report(const SourceLocation& location, Kind, const Error& error)
{
    auto code = toCode(error);
    auto module = toModule(error);
    auto moduleName = toModuleName(error);
    auto errorName = toErrorName(error);

    IOX_ERROR_INTERNAL_LOG(location) << ": " << errorName << " (code " << code.value << ") in module " << moduleName
                                     << " (id " << module.value << ")";
    auto& h = ErrorHandler::get();
    h.onReportError(ErrorDescriptor(location, code, module));
}

// Report any error, specialization for specific types overrides the general version.
// Any behaviour for specific error types (and kinds) has to be defined like this.
//
// Here the logging is subtly different and does not easily allow to factor out common parts.

template <class Error>
inline void report(const SourceLocation& location, iox::er::FatalKind kind, const Error& error)
{
    auto code = toCode(error);
    auto module = toModule(error);
    auto moduleName = toModuleName(error);
    auto errorName = toErrorName(error);

    IOX_ERROR_INTERNAL_LOG_FATAL(location) << ": " << kind.name << " " << errorName << " (code " << code.value
                                           << ") in module " << moduleName << " (id " << module.value << ")";
    auto& h = ErrorHandler::get();
    h.onReportError(ErrorDescriptor(location, code, module));
}

template <class Error>
inline void report(const SourceLocation& location, iox::er::PreconditionViolationKind kind, const Error& error)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_ERROR_INTERNAL_LOG_FATAL(location) << kind.name;
    auto& h = ErrorHandler::get();
    h.onReportViolation(ErrorDescriptor(location, code, module));
}

template <class Error>
inline void report(const SourceLocation& location, iox::er::AssumptionViolationKind kind, const Error& error)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_ERROR_INTERNAL_LOG_FATAL(location) << kind.name;
    auto& h = ErrorHandler::get();
    h.onReportViolation(ErrorDescriptor(location, code, module));
}

template <class Error, class Message>
inline void
report(const SourceLocation& location, iox::er::PreconditionViolationKind kind, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_ERROR_INTERNAL_LOG_FATAL(location) << kind.name << " " << std::forward<Message>(msg);
    auto& h = ErrorHandler::get();
    h.onReportViolation(ErrorDescriptor(location, code, module));
}

template <class Error, class Message>
inline void
report(const SourceLocation& location, iox::er::AssumptionViolationKind kind, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_ERROR_INTERNAL_LOG_FATAL(location) << kind.name << " " << std::forward<Message>(msg);
    auto& h = ErrorHandler::get();
    h.onReportViolation(ErrorDescriptor(location, code, module));
}

} // namespace er
} // namespace iox

#endif
