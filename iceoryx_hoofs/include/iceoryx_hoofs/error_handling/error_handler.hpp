// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_HPP
#define IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_HPP

#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/log/logger.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logmanager.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
namespace iox
{
/// @brief the available error levels
/// FATAL
/// - Log message with FATAL
/// - RouDi cannot recover from that error. RouDi is terminated
/// - Assert (in DEBUG) and terminate
/// - Reporting code must handle this and continue or go to a save state. Error handler could return (e.g. in test)
/// SEVERE
/// - Log message with ERROR
/// - RouDi can still run. Error is reported
/// - Assert in DEBUG, in RELEASE continue to run
/// - Reporting code must handle this and continue
/// MODERATE
/// - Log message with ERROR
/// - RouDi can still run. Error is reported
/// - NO assert
/// - Reporting code must handle this and continue
enum class ErrorLevel : uint32_t
{
    /// Log error entry + Assert + terminate
    FATAL,
    /// warning log entry + Assert
    SEVERE,
    /// warning log entry
    MODERATE
};

template <typename Error>
using HandlerFunction = std::function<void(const Error error, const std::function<void()>, const ErrorLevel)>;

/// @brief This handler is needed for unit testing, special debugging cases and
///         other corner cases where we'd like to explicitly suppress the
///         error handling.
template <typename Error>
class ErrorHandler
{
    template <typename ErrorType, typename>
    friend void
    errorHandler(const ErrorType error, const std::function<void()>& errorCallBack, const ErrorLevel level) noexcept;

  public:
    static cxx::GenericRAII setTemporaryErrorHandler(const HandlerFunction<Error>& newHandler) noexcept;

  protected:
    static void reactOnErrorLevel(const ErrorLevel level, const char* errorText) noexcept;

  private:
    static void defaultHandler(const Error error,
                               const std::function<void()>& errorCallBack,
                               const ErrorLevel level = ErrorLevel::FATAL) noexcept;

    static iox::HandlerFunction<Error> handler;
    /// Needed, if you want to exchange the handler. Remember the old one and call it if it is not your error. The error
    /// mock needs to be the last one exchanging the handler in tests.
    static std::mutex handler_mutex;
};

/// @brief Howto use the error handler correctly
///     1.) If the error you would like to handle is not listed in ICEORYX_ERRORS(error)\...
///             macro just add them like:
///             error(MODULE_NAME__MY_FUNKY_ERROR)
///         Attention: Create an error after the following convention:
///             MODULE_NAME__A_CLEAR_BUT_SHORT_ERROR_DESCRIPTION
///         And a long name is alright!
///
///     2.) Call errorHandler(Error::kMODULE_NAME__MY_FUNKY_ERROR);
///             Please pay attention to the "k" prefix
///         The defaults for errorCallback and ErrorLevel can also be overwritten:
///             errorHandler(
///                 Error::kMODULE_NAME__MY_FUNKY_ERROR,
///                 []{ std::cout << "MyCustomCallback" << std::endl; },
///                 ErrorLevel::MODERATE
///             );
///
/// @code
/// class PrettyClass {
///     float division(float a, float b) {
///         if ( b == 0.0f ) {
///             errorHandler(Error::kPRETTY_CLASS__DIVISION_BY_ZERO);
///         }
///     }
/// };
/// @endcode
///
/// @code
/// bool called = false;
/// auto temporaryErrorHandler = ErrorHandler::setTemporaryErrorHandler(
///     [&](const Error e, std::function<void()>, const ErrorLevel) {
///         called = true;
///     });
///
/// errorHandler(Error::kTEST__ASSERT_CALLED);
/// ASSERT_TRUE(called);
/// @endcode
template <typename Error, typename = std::enable_if_t<std::is_enum<Error>::value>>
inline void errorHandler(const Error error,
                         const std::function<void()>& errorCallBack = std::function<void()>(),
                         const ErrorLevel level = ErrorLevel::FATAL) noexcept
{
    ErrorHandler<Error>::handler(error, errorCallBack, level);
}


} // namespace iox

#include "iceoryx_hoofs/internal/error_handling/error_handler.inl"

#endif // IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_HPP