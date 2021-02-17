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

#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"

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
{
    *this = std::move(rhs);
}

SignalGuard::~SignalGuard() noexcept
{
    restorePreviousAction();
}

SignalGuard& SignalGuard::operator=(SignalGuard&& rhs) noexcept
{
    if (this != &rhs)
    {
        restorePreviousAction();

        m_signal = std::move(rhs.m_signal);
        m_previousAction = std::move(rhs.m_previousAction);
        m_doRestorePreviousAction = std::move(rhs.m_doRestorePreviousAction);

        rhs.m_doRestorePreviousAction = false;
    }
    return *this;
}

void SignalGuard::restorePreviousAction() noexcept
{
    if (m_doRestorePreviousAction)
    {
        m_doRestorePreviousAction = false;
        if (cxx::makeSmartC(sigaction,
                            cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE,
                            {0},
                            {},
                            static_cast<int>(m_signal),
                            &m_previousAction,
                            nullptr)
                .hasErrors())
        {
            std::cerr << "Unable to restore the previous signal handling state!" << std::endl;
        }
    }
}


SignalGuard registerSignalHandler(const Signal signal, const SignalHandlerCallback_t callback) noexcept
{
    struct sigaction action;

    if (cxx::makeSmartC(sigemptyset, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, &action.sa_mask).hasErrors())
    {
        std::cerr << "Unable to create an empty sigaction set while registering signal handler. No signal handler will "
                     "be registered!"
                  << std::endl;
        return SignalGuard();
    }

    action.sa_handler = callback;
    action.sa_flags = 0;

    struct sigaction previousAction;

    if (cxx::makeSmartC(sigaction,
                        cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE,
                        {0},
                        {},
                        static_cast<int>(signal),
                        &action,
                        &previousAction)
            .hasErrors())
    {
        std::cerr << "This should never happen! An error occurred while registering a signal handler." << std::endl;
    }

    return SignalGuard(signal, previousAction);
}
} // namespace posix
} // namespace iox
