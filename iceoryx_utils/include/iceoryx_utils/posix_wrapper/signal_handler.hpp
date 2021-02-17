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
#ifndef IOX_UTILS_POSIX_WRAPPER_SIGNAL_HANDLER_HPP
#define IOX_UTILS_POSIX_WRAPPER_SIGNAL_HANDLER_HPP

#include <csignal>

namespace iox
{
namespace posix
{
using SignalHandlerCallback_t = void (*)(int);

enum class Signal : int
{
    BUS = SIGBUS,
    INT = SIGINT,
    TERM = SIGTERM
    /// @attention never add SIGKILL or SIGSTOP into this list, they cannot be caught
    ///            and sigaction returns the errno EINVAL
};

class SignalGuard
{
  public:
    SignalGuard() noexcept = default;
    SignalGuard(const Signal signal, const struct sigaction& previousAction) noexcept;
    SignalGuard(const SignalGuard&) = delete;
    SignalGuard(SignalGuard&& rhs) noexcept;
    ~SignalGuard() noexcept;

    SignalGuard& operator=(const SignalGuard& rhs) = delete;
    SignalGuard& operator=(SignalGuard&& rhs) noexcept;

  private:
    void restorePreviousAction() noexcept;

  private:
    Signal m_signal;
    struct sigaction m_previousAction;
    bool m_doRestorePreviousAction{false};
};

SignalGuard registerSignalHandler(const Signal signal, const SignalHandlerCallback_t callback) noexcept;
} // namespace posix
} // namespace iox
#endif

