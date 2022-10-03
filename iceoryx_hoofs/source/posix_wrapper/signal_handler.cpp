// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/signal_handler.hpp"
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace posix
{
SignalGuard::SignalGuard(const Signal signal, const struct sigaction& previousAction) noexcept
    : m_signal{signal}
    , m_previousAction{previousAction}
    , m_doRestorePreviousAction(true)
{
}

SignalGuard::SignalGuard(SignalGuard&& rhs) noexcept
    : m_signal{rhs.m_signal}
    , m_previousAction{rhs.m_previousAction}
    , m_doRestorePreviousAction{rhs.m_doRestorePreviousAction}
{
    rhs.m_doRestorePreviousAction = false;
}

SignalGuard::~SignalGuard() noexcept
{
    restorePreviousAction();
}

void SignalGuard::restorePreviousAction() noexcept
{
    if (m_doRestorePreviousAction)
    {
        m_doRestorePreviousAction = false;
        posixCall(sigaction)(static_cast<int>(m_signal), &m_previousAction, nullptr)
            .successReturnValue(0)
            .evaluate()
            .or_else([](auto&) { std::cerr << "Unable to restore the previous signal handling state!" << std::endl; });
    }
}

cxx::expected<SignalGuard, SignalGuardError> registerSignalHandler(const Signal signal,
                                                                   const SignalHandlerCallback_t callback) noexcept
{
    struct sigaction action = {};


    // sigemptyset fails when a nullptr is provided and this should never happen with this logic
    if (posixCall(sigemptyset)(&action.sa_mask).successReturnValue(0).evaluate().has_error())
    {
        LogError() << "This should never happen! Unable to create an empty sigaction set while registering a signal "
                      "handler for the signal ["
                   << static_cast<int>(signal) << "]. No signal handler will be registered!";
        return cxx::error<SignalGuardError>(SignalGuardError::INVALID_SIGNAL_ENUM_VALUE);
    }

    // system struct, no way to avoid union
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    action.sa_handler = callback;
    action.sa_flags = 0;


    struct sigaction previousAction = {};

    // sigaction fails when action is a nullptr (which we ensure that its not) or when the signal SIGSTOP or SIGKILL
    // should be registered which can also never happen - ensured by the enum class.
    if (posixCall(sigaction)(static_cast<int>(signal), &action, &previousAction)
            .successReturnValue(0)
            .evaluate()
            .has_error())
    {
        LogError() << "This should never happen! An error occurred while registering a signal handler for the signal ["
                   << static_cast<int>(signal) << "]. ";
        return cxx::error<SignalGuardError>(SignalGuardError::UNDEFINED_ERROR_IN_SYSTEM_CALL);
    }

    return cxx::success<SignalGuard>(SignalGuard(signal, previousAction));
}
} // namespace posix
} // namespace iox
