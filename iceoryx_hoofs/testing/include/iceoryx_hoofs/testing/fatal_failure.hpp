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

#ifndef IOX_HOOFS_TESTING_FATAL_FAILURE_HPP
#define IOX_HOOFS_TESTING_FATAL_FAILURE_HPP

#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "iceoryx_hoofs/testing/mocks/error_handler_mock.hpp"
#include "iox/optional.hpp"
#include "test.hpp"

#include <atomic>
#include <thread>

// NOLINTNEXTLINE(hicpp-deprecated-headers) required to work on some platforms
#include <setjmp.h>

namespace iox
{
namespace testing
{
namespace detail
{
/// @brief This function is the base for 'IOX_EXPECT_FATAL_FAILURE' and 'IOX_EXPECT_NO_FATAL_FAILURE' and should not be
/// used by its own. The function only works in combination with the iceoryx error handler.
/// @tparam[in] ErrorType The error type which is expected, e.g. 'iox::HoofsError'
/// @param[in] testFunction This function will be executed as SUT and might call the error handler with a 'FATAL' error
/// level
/// @param[in] onFatalFailurePath This function will be executed on the failure path after the failure was detected
/// @param[in] onNonFatalFailurePath This function will be executed on the non-failure path if no failure was detected
/// @return true if a fatal failure occurs, false otherwise
template <typename ErrorType>
bool IOX_FATAL_FAILURE_TEST(const cxx::function_ref<void()> testFunction,
                            const cxx::function_ref<void(const ErrorType, const iox::ErrorLevel)> onFatalFailurePath,
                            const cxx::function_ref<void()> onNonFatalFailurePath);
} // namespace detail

/// @brief This function is used in cases a fatal failure is expected. The function only works in combination with the
/// iceoryx error handler.
/// @code
/// TEST(MyTest, valueOnNulloptIsFatal) {
///     iox::optional<bool> sut;
///     IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.value(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED));
/// }
/// @endcode
/// @tparam[in] ErrorType The error type which is expected, e.g. 'iox::HoofsError'
/// @param[in] testFunction This function will be executed as SUT and is expected to call the error handler
/// @param[in] expectedError The error value which triggered the fatal failure
/// @return true if a fatal failure occurs, false otherwise
template <typename ErrorType>
bool IOX_EXPECT_FATAL_FAILURE(const cxx::function_ref<void()> testFunction, const ErrorType expectedError);

/// @brief This function is used in cases no fatal failure is expected but could potentially occur. The function only
/// works in combination with the iceoryx error handler.
/// @code
/// TEST(MyTest, valueIsNotFatal) {
///     iox::optional<bool> sut{false};
///     IOX_EXPECT_NO_FATAL_FAILURE<iox::HoofsError>([&] { sut.value(); });
/// }
/// @endcode
/// @tparam[in] ErrorType The error type which is expected if the test fails, e.g. 'iox::HoofsError'
/// @param[in] testFunction This function will be executed as SUT and is not expected to call the error handler
/// @return true if no fatal failure occurs, false otherwise
template <typename ErrorType>
bool IOX_EXPECT_NO_FATAL_FAILURE(const cxx::function_ref<void()> testFunction);

} // namespace testing
} // namespace iox


#include "iceoryx_hoofs/testing/fatal_failure.inl"

#endif // IOX_HOOFS_TESTING_FATAL_FAILURE_HPP
