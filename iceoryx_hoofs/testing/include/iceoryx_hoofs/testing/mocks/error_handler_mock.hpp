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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include "iceoryx_hoofs/testing/test.hpp"

namespace iox
{
template <typename Error>
using TypedHandlerFunction = std::function<void(const Error, const ErrorLevel)>;

class ErrorHandlerMock : protected ErrorHandler
{
  public:
    template <typename Error>
    static cxx::GenericRAII setTemporaryErrorHandler(const TypedHandlerFunction<Error>& newHandler) noexcept;

  protected:
    /// Needed, if you want to exchange the handler. Remember the old one and call it if it is not your error. The error
    /// mock needs to be the last one exchanging the handler in tests.
    static std::mutex handler_mutex;
};

template <typename Error>
cxx::optional<iox::TypedHandlerFunction<Error>> typedHandler;

template <typename ErrorEnumType>
inline void errorHandlerForTest(const uint32_t error, const char* errorName, const ErrorLevel level) noexcept
{
    uint32_t errorEnumType = error >> 16;
    uint32_t expectedErrorEnumType =
        static_cast<typename std::underlying_type<ErrorEnumType>::type>(ErrorEnumType::kNO_ERROR) >> 16;

    if (errorEnumType == expectedErrorEnumType)
    {
        // We undo the type erasure
        auto typedError = static_cast<ErrorEnumType>(error);
        typedHandler<iox::Error>.and_then(
            [&](TypedHandlerFunction<Error> storedHandler) { storedHandler(typedError, level); });
    }
    else
    {
        GTEST_FAIL() << "errorName: " << errorName << ", expected error enum type: " << expectedErrorEnumType
                     << ", actual error enum type: " << errorEnumType;
    }
}

template <typename Error>
inline cxx::GenericRAII
ErrorHandlerMock::setTemporaryErrorHandler(const TypedHandlerFunction<Error>& newHandler) noexcept
{
    return cxx::GenericRAII(
        [&newHandler] {
            std::lock_guard<std::mutex> lock(handler_mutex);
            typedHandler<iox::Error>.emplace(newHandler);
            handler = errorHandlerForTest<Error>;
        },
        [] {
            std::lock_guard<std::mutex> lock(handler_mutex);
            typedHandler<iox::Error>.reset();
            handler = defaultHandler;
        });
}
} // namespace iox
#endif // IOX_HOOFS_TESTUTILS_ERROR_HANDLER_MOCK_HPP
