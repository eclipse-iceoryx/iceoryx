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

#ifndef IOX_HOOFS_TESTING_ERROR_REPORTING_TEST_ERROR_HANDLER_HPP
#define IOX_HOOFS_TESTING_ERROR_REPORTING_TEST_ERROR_HANDLER_HPP

#include "iceoryx_hoofs/error_reporting/custom/default/error_handler_interface.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/errors.hpp"
#include "iceoryx_hoofs/error_reporting/source_location.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

#include <atomic>
#include <vector>

// we can use this for test code
#include <mutex>

// NOLINTNEXTLINE(hicpp-deprecated-headers) required to work on some platforms
#include <setjmp.h>

namespace iox
{
namespace testing
{

/// @brief Defines the test reaction of dynamic error handling.
class TestErrorHandler : public iox::err::ErrorHandlerInterface
{
  public:
    TestErrorHandler();

    ~TestErrorHandler() override = default;
    TestErrorHandler(const TestErrorHandler&) = delete;
    TestErrorHandler(TestErrorHandler&&) = delete;
    TestErrorHandler& operator=(const TestErrorHandler&) = delete;
    TestErrorHandler operator=(TestErrorHandler&&) = delete;

    /// @brief Defines the reaction on panic.
    void onPanic() override;

    /// @brief Defines the reaction on error.
    /// @param desc error descriptor
    void onReportError(err::ErrorDescriptor desc) override;

    /// @brief Defines the reaction on violation.
    /// @param desc error descriptor
    void onReportViolation(err::ErrorDescriptor desc) override;

    /// @brief Indicates whether there was a panic call previously.
    /// @return true if there was a panic call, false otherwise
    bool hasPanicked() const;

    /// @brief Reset panic state and clears all errors that occurred previously.
    void reset();

    /// @brief Indicates whether any error occurred previously.
    bool hasError() const;

    /// @brief Indicates whether a specific error occurred previously.
    bool hasError(iox::err::ErrorCode code, iox::err::ModuleId module = iox::err::ModuleId()) const;

    /// @brief Indicates whether a assumption violation occurred previously.
    /// @note We do not track module id for violations.
    bool hasViolation(iox::err::ErrorCode code) const;

    /// @brief Prepare a jump and return jump buffer
    /// @return pointer to jump buffer if successful, nullptr otherwise
    jmp_buf* prepareJump();

    /// @brief Returns the value that is set by longjmp in case of a jump.
    /// @return the jump indicator value
    static int jumpIndicator();

  private:
    static constexpr int JUMPED{1};

    mutable std::mutex m_mutex;
    std::atomic<bool> m_panicked{false};
    std::vector<err::ErrorDescriptor> m_errors;

    // we track violations separately (leads to simple search)
    std::vector<err::ErrorDescriptor> m_violations;

    // if we would like to support concurrent jumps it gets very tricky
    // and we would need multiple jump buffers
    jmp_buf m_jumpBuffer{};

    // Actually not needed to be atomic since it is not supposed to be used from multiple threads
    // (longjmp does not support this)
    // We need to ensure though that only one jump buffer is considered by panic and controlling
    // ownership of the buffer is one way to accomplish that.
    std::atomic<jmp_buf*> m_jump{nullptr};

    void jump();
};

} // namespace testing
} // namespace iox

#endif
