// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/signal_handler.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
SignalGuard::SignalGuard(const PosixSignal signal, const struct sigaction& previousAction) noexcept
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
        IOX_POSIX_CALL(sigaction)
        (static_cast<int>(m_signal), &m_previousAction, nullptr).successReturnValue(0).evaluate().or_else([](auto&) {
            IOX_LOG(ERROR, "Unable to restore the previous signal handling state!");
        });
    }
}

expected<SignalGuard, SignalGuardError> registerSignalHandler(const PosixSignal signal,
                                                              const SignalHandlerCallback_t callback) noexcept
{
    struct sigaction action = {};


    // sigemptyset fails when a nullptr is provided and this should never happen with this logic
    if (IOX_POSIX_CALL(sigemptyset)(&action.sa_mask).successReturnValue(0).evaluate().has_error())
    {
        IOX_LOG(ERROR,
                "This should never happen! Unable to create an empty sigaction set while registering a signal handler "
                "for the signal ["
                    << static_cast<int>(signal) << "]. No signal handler will be registered!");
        return err(SignalGuardError::INVALID_SIGNAL_ENUM_VALUE);
    }

    // system struct, no way to avoid union
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    action.sa_handler = callback;
    action.sa_flags = 0;


    struct sigaction previousAction = {};

    // sigaction fails when action is a nullptr (which we ensure that its not) or when the signal SIGSTOP or SIGKILL
    // should be registered which can also never happen - ensured by the enum class.
    if (IOX_POSIX_CALL(sigaction)(static_cast<int>(signal), &action, &previousAction)
            .successReturnValue(0)
            .evaluate()
            .has_error())
    {
        IOX_LOG(ERROR,
                "This should never happen! An error occurred while registering a signal handler for the signal ["
                    << static_cast<int>(signal) << "]. ");
        return err(SignalGuardError::UNDEFINED_ERROR_IN_SYSTEM_CALL);
    }

    return ok(SignalGuard(signal, previousAction));
}
} // namespace iox
