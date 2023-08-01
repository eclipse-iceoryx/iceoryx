// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_TESTUTILS_ERROR_HANDLER_MOCK_HPP
#define IOX_HOOFS_TESTUTILS_ERROR_HANDLER_MOCK_HPP

#include "iceoryx_hoofs/error_handling/error_handler.hpp"
#include "iox/function.hpp"
#include "iox/scope_guard.hpp"
#include <mutex>

#include "iceoryx_hoofs/testing/test.hpp"

namespace iox
{
template <typename Error>
using TypedHandlerFunction = function<void(const Error, const ErrorLevel)>;

/// @brief This mock is needed for unit testing, special debugging cases and
///         other corner cases where we'd like to explicitly suppress the
///         error handling.
///
/// @code
/// bool called = false;
/// auto temporaryErrorHandler = ErrorHandlerMock::setTemporaryErrorHandler<Error>(
///     [&](const Error, const ErrorLevel) {
///         called = true;
///     });
///
/// errorHandler(Error::TEST__ASSERT_CALLED);
/// ASSERT_TRUE(called);
/// @endcode
class ErrorHandlerMock : protected ErrorHandler
{
  public:
    template <typename Error>
    static ScopeGuard setTemporaryErrorHandler(const TypedHandlerFunction<Error>& newHandler) noexcept;

  protected:
    static std::mutex m_handlerMutex;
};

template <typename Error>
optional<iox::TypedHandlerFunction<Error>> typedHandler;

template <typename ErrorEnumType>
inline void errorHandlerForTest(const uint32_t error, const char* errorName, const ErrorLevel level) noexcept
{
    uint32_t errorModuleIdentifier = error >> ERROR_ENUM_OFFSET_IN_BITS;
    uint32_t expectedErrorModuleIdentifier =
        static_cast<typename std::underlying_type<ErrorEnumType>::type>(ErrorEnumType::NO_ERROR)
        >> ERROR_ENUM_OFFSET_IN_BITS;

    if (errorModuleIdentifier == expectedErrorModuleIdentifier)
    {
        // We undo the type erasure
        auto typedError = static_cast<ErrorEnumType>(error);
        typedHandler<ErrorEnumType>.and_then(
            [&](TypedHandlerFunction<ErrorEnumType> storedHandler) { storedHandler(typedError, level); });
    }
    else
    {
        GTEST_FAIL() << "errorName: " << errorName << ", expected error enum type: " << expectedErrorModuleIdentifier
                     << ", actual error enum type: " << errorModuleIdentifier;
    }
}

template <typename Error>
inline ScopeGuard ErrorHandlerMock::setTemporaryErrorHandler(const TypedHandlerFunction<Error>& newHandler) noexcept
{
    return ScopeGuard(
        [&newHandler] {
            std::lock_guard<std::mutex> lock(m_handlerMutex);
            typedHandler<Error>.emplace(newHandler);
            handler = errorHandlerForTest<Error>;
        },
        [] {
            std::lock_guard<std::mutex> lock(m_handlerMutex);
            typedHandler<Error>.reset();
            handler = defaultHandler;
        });
}
} // namespace iox
#endif // IOX_HOOFS_TESTUTILS_ERROR_HANDLER_MOCK_HPP
