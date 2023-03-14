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

#include "iceoryx_hoofs/testing/error_reporting/test_error_handler.hpp"
#include "iceoryx_hoofs/error_reporting/custom/default/error_handler_interface.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

// NOLINTNEXTLINE(hicpp-deprecated-headers) required to work on some platforms
#include <setjmp.h>

#include <iostream>

namespace iox
{
namespace testing
{

using namespace iox::err;

TestErrorHandler::TestErrorHandler()
    : m_jump(&m_jumpBuffer)
{
}

void TestErrorHandler::onPanic()
{
    m_panicked = true;
    jump();
}

void TestErrorHandler::onReportError(err::ErrorDescriptor desc)
{
    std::lock_guard<std::mutex> g(m_mutex);
    m_errors.push_back(desc);
}

void TestErrorHandler::onReportViolation(err::ErrorDescriptor desc)
{
    std::lock_guard<std::mutex> g(m_mutex);
    m_violations.push_back(desc);
}

bool TestErrorHandler::hasPanicked() const
{
    return m_panicked;
}

void TestErrorHandler::reset()
{
    std::lock_guard<std::mutex> g(m_mutex);
    m_panicked = false;
    m_errors.clear();
    m_violations.clear();
    m_jump.store(&m_jumpBuffer);
}

bool TestErrorHandler::hasError() const
{
    std::lock_guard<std::mutex> g(m_mutex);
    return !m_errors.empty();
}

bool TestErrorHandler::hasError(ErrorCode code, iox::err::ModuleId module) const
{
    constexpr iox::err::ModuleId ANY_MODULE{iox::err::ModuleId::ANY};
    std::lock_guard<std::mutex> g(m_mutex);
    for (auto desc : m_errors)
    {
        if (desc.code == code)
        {
            if (module == ANY_MODULE)
            {
                return true;
            }
            return desc.module == module;
        }
    }
    return false;
}

bool TestErrorHandler::hasViolation(ErrorCode code) const
{
    std::lock_guard<std::mutex> g(m_mutex);
    for (auto desc : m_violations)
    {
        if (desc.code == code)
        {
            return true;
        }
    }
    return false;
}

jmp_buf* TestErrorHandler::prepareJump()
{
    // winner can prepare the jump
    return m_jump.exchange(nullptr);
}

void TestErrorHandler::jump()
{
    jmp_buf* exp = nullptr;
    // if it is a nullptr, somebody (and only one) has prepared jump
    // it will be reset on first jump, so there cannot be concurrent jumps
    // essentially the first panic call wins, resets and and jumps
    if (m_jump.compare_exchange_strong(exp, &m_jumpBuffer))
    {
        // NOLINTNEXTLINE(cert-err52-cpp) exception handling is not used by design
        longjmp(&m_jumpBuffer[0], jumpIndicator());
    }
}

int TestErrorHandler::jumpIndicator()
{
    return JUMPED;
}

} // namespace testing
} // namespace iox
