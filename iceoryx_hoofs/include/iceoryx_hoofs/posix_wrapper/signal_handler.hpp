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
#ifndef IOX_HOOFS_POSIX_WRAPPER_SIGNAL_HANDLER_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SIGNAL_HANDLER_HPP

#include "iceoryx_platform/signal.hpp"
#include "iox/expected.hpp"

namespace iox
{
namespace posix
{
using SignalHandlerCallback_t = void (*)(int);

/// @brief Corresponds to the SIG* macros defined in signal.h. The integer values
///        are equal to the corresponding macro value.
enum class Signal : int
{
    BUS = SIGBUS,
    INT = SIGINT,
    TERM = SIGTERM,
    HUP = SIGHUP,
    ABORT = SIGABRT,
    /// @attention never add SIGKILL or SIGSTOP into this list, they cannot be caught
    ///            and sigaction returns the errno EINVAL
};

enum class SignalGuardError
{
    INVALID_SIGNAL_ENUM_VALUE,
    UNDEFINED_ERROR_IN_SYSTEM_CALL
};

/// @attention NEVER USE THIS CLASS AS A MEMBER VARIABLE! A class which should be used
///            only in method/function scopes.
/// @brief The SignalGuard is a class returned by registerSignalHandler. When it goes
///         out of scope it restores the previous signal action. Typical use case:
///         One would like to override the signal action in main() or some C posix
///         makes it necessary to override the standard signal action before and
///         after the call.
/// @code
///    {
///      auto signalGuard = registerSignalHandler(Signal::BUS, printErrorMessage);
///      my_c_call_which_can_cause_SIGBUS();
///    }
///    // here we are out of scope and the signal action for Signal::BUS is restored
/// @endcode
class SignalGuard
{
  public:
    SignalGuard(SignalGuard&& rhs) noexcept;
    SignalGuard(const SignalGuard&) = delete;
    ~SignalGuard() noexcept;

    SignalGuard& operator=(const SignalGuard& rhs) = delete;
    SignalGuard& operator=(SignalGuard&& rhs) = delete;

    friend expected<SignalGuard, SignalGuardError> registerSignalHandler(const Signal,
                                                                         const SignalHandlerCallback_t) noexcept;

  private:
    void restorePreviousAction() noexcept;
    SignalGuard(const Signal signal, const struct sigaction& previousAction) noexcept;

  private:
    Signal m_signal;
    struct sigaction m_previousAction = {};
    bool m_doRestorePreviousAction{false};
};

/// @brief Register a callback for a specific posix signal (SIG***).
/// @attention if a signal callback was already registered for the provided signal with registerSignalHandler or
///             with sigaction() or signal(), the signal callback is overridden
///             until the SignalGuard goes out of scope and restores the previous callback. If you override the
///             callbacks multiple times and the created SignalGuards goes out of scope in a different order then the
///             callback is restored which was active when the last SignalGuard which is going out of scope was created.
/// @param[in] Signal the signal to which the callback should be attached
/// @param[in] callback the callback which should be called when the signal is raised.
/// @return SignalGuard on success - when it goes out of scope the previous signal action is restored. On error
///         SignalGuardError is returned which describes the error.
expected<SignalGuard, SignalGuardError> registerSignalHandler(const Signal signal,
                                                              const SignalHandlerCallback_t callback) noexcept;
} // namespace posix
} // namespace iox
#endif
