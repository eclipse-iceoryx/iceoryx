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

#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_REPORTING_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"
#include "iceoryx_hoofs/error_reporting/platform/error_kind.hpp"

namespace iox
{
namespace err
{

// The static reporting interface that must be defined to at least do nothing
// This implementation redirects to the polymorphic handler interface.
// This adds an additional indirection but is required for testing or switching handlers
// during operation (this must be done very carefully and is not recommended).

[[noreturn]] inline void panic(const SourceLocation& location)
{
    IOX_LOG_PANIC(location) << "Panic";
    auto& h = ErrorHandler::get();
    h.panic();
    abort();
}

[[noreturn]] inline void panic(const SourceLocation& location, const char* msg)
{
    IOX_LOG_PANIC(location) << "Panic " << msg;
    auto& h = ErrorHandler::get();
    h.panic();
    abort();
}

template <class Kind, class Error>
inline void report(const SourceLocation& location, Kind, const Error& error)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_LOG_ERROR(location) << " Error " << code.value << " in module " << module.value;
    auto& h = ErrorHandler::get();
    h.reportError(ErrorDescriptor(location, code, module));
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::Fatal, const Error& error)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_LOG_FATAL_ERROR(location) << " Fatal Error " << code.value << " in module " << module.value;
    auto& h = ErrorHandler::get();
    h.reportError(ErrorDescriptor(location, code, module));
}

template <class Error, class Message>
inline void report(const SourceLocation& location, iox::err::PreconditionViolation, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_LOG_FATAL_ERROR(location) << ": Precondition Violation " << std::forward<Message>(msg);
    auto& h = ErrorHandler::get();
    h.reportViolation(ErrorDescriptor(location, code, module));
}

template <class Error, class Message>
inline void report(const SourceLocation& location, iox::err::DebugAssertViolation, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    auto module = toModule(error);
    IOX_LOG_FATAL_ERROR(location) << ": Debug Assert Violation " << std::forward<Message>(msg);
    auto& h = ErrorHandler::get();
    h.reportViolation(ErrorDescriptor(location, code, module));
}

} // namespace err
} // namespace iox

#endif
