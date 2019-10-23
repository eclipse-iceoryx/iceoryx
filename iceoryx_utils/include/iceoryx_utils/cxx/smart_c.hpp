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

#include <cstring>
#include <initializer_list>
#include <iostream>

/// @brief Always use this macro to create smart_c objects.
/// @param[in] f_function Function you wish to call
/// @param[in] f_returnValues Possible return values that this function can return
/// @param[in] f_ignoredValues Possible return values that this function can return, but is not blaming them in the log.
/// @param[in] ... Arguments which will be passed to the function
///
/// @brief Use the macro instead of this direct function call. The macro fills
/// automatically the source of the function call via the __FILE__, __LINE__
/// and __PRETTY_FUNCTION__ defines.
/// Creates a smart_c c function call and executes the call. Depending on
/// how successful the c function call was it either returns via std::cerr an
/// error message containing the source of this make_smart_c or it stores the
/// return value. We can retrieve it later by calling GetReturnValue() or
/// casting since smart_c has an casting operator to the returnValue.
///
/// @code
///     auto memoryCall =
///         cxx::make_smart_c(malloc, cxx::returnMode::PreDefinedErrorCode, {static_cast< void* >(nu
///         >(nullptr)}, 10); 10);
///         {}, 10); void * pointer; if ( !memoryCall.HasErrors() ) {
///             pointer = memoryCall.GetReturnValue();
///
///             // it is also possible to assign it directly since it has an
///             // cast operator to the ReturnType
///             pointer = memoryCall;
///         }
/// @endcode
#define makeSmartC(f_function, f_returnMode, f_returnValues, f_ignoredValues, ...)                                     \
    makeSmartCImpl(__FILE__,                                                                                           \
                   __LINE__,                                                                                           \
                   __PRETTY_FUNCTION__,                                                                                \
                   f_function,                                                                                         \
                   f_returnMode,                                                                                       \
                   f_returnValues,                                                                                     \
                   f_ignoredValues,                                                                                    \
                   __VA_ARGS__)

/// @todo c function with only one valid value and an infinite number of invalid values are not useable with smart_c

namespace iox
{
namespace cxx
{
/// @brief Defined the return code behavior of a c function. Does the function
///         has a specific code on success and an arbitrary number of error codes
///         or does it have a specific code on error and an arbitrary number of
///         success codes.
enum class ReturnMode
{
    /// @brief The function has a specific code on success
    PRE_DEFINED_SUCCESS_CODE,
    /// @brief The function has a specific code on error
    PRE_DEFINED_ERROR_CODE
};
/// @brief C function call abstraction class which performs the error handling
///         automatically.
/// @code
///     #include <cstdlib>
///     #include "smart_c.hpp"
///
///     auto memoryCall =
///         cxx::make_smart_c(malloc, cxx::returnModenModenModenMode::PreDefinedErrorCode, {static_cast< void*
///         >( 10l; 10);
///         {}, 10); void * pointer; if ( !memoryCall.HasErrors() ) {
///             pointer = memoryCall.GetReturnValue();
///         }
///
///     ...
///
///     auto semaphoreCall  =
///         cxx::make_smart_c(sem_open, cxx::returnModenMode::PreDefinedErrorCode, {SEM_FAILED}, {}, "param1",
///         12);
///
///     // if an error has occurred the optional has no value
///     if ( semaphoreCall.HasErrors() ) {
///         DoStuffWithSemaphore(semaphore.GetReturnValue());
///     }
/// @endcode
template <typename Function, typename ReturnType, typename... FunctionArguments>
class SmartC
{
  public:
    /// @brief Returns the returnValue of the c function call. If an error
    ///         has occurred the error code is returned.
    ///         If you use it in your code you should probably check with
    ///         HasErrors() if an actual error has occurred during the call.
    /// @return returnValue of the c call
    ReturnType getReturnValue() const noexcept;

    /// @brief conversion operator to the return type of the c call
    /// @return returnValue of the c call
    operator ReturnType() const noexcept;

    /// @brief If one of the given error codes was returned during the c
    ///         function call and the c function failed it returns false,
    ///         otherwise true
    /// @return false if the c call failed, otherwise true
    bool hasErrors() const noexcept;

    /// @brief If no error occurred it returns a string like "no errors"
    ///         (depending on the posix system) otherwise it returns the
    ///         errnum error string
    /// @return if the c call failed the result of strerror(errno)
    const char* getErrorString() const noexcept;

    /// @brief Returns the errnum. 0 if no error has occurred, otherwise != 0
    /// @return returns the errno value which was set by the c call
    int getErrNum() const noexcept;

    template <typename Function_F, typename ReturnType_F, typename... FunctionArguments_F>
    friend SmartC<Function_F, ReturnType_F, FunctionArguments_F...>
    makeSmartCImpl(const char* file,
                   const int line,
                   const char* func,
                   const Function_F& f_function,
                   const ReturnMode& f_mode,
                   const std::initializer_list<ReturnType_F>& f_returnValues,
                   const std::initializer_list<int>& f_ignoredValues,
                   FunctionArguments_F... f_args) noexcept;

  private:
    /// @brief You should never create a smart c object directly and should
    /// always use make_SmartC, therefore the ctor is private
    SmartC(const char* file,
           const int line,
           const char* func,
           const Function& f_function,
           const ReturnMode& f_mode,
           const std::initializer_list<ReturnType>& f_returnValues,
           const std::initializer_list<int>& f_ignoredValues,
           FunctionArguments... f_args) noexcept;

    int resetErrnoAndInitErrnum() noexcept;

  private:
    static constexpr int m_errorStringSize = 128;
    int m_errnum = 0;
    ReturnType m_returnValue;
    char m_errorString[m_errorStringSize];
    bool m_hasErrors = false;

    struct
    {
        const char* file;
        int line;
        const char* func;
    } m_errorSource;
};
} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/smart_c.inl"
