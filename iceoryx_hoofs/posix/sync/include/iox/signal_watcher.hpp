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

#ifndef IOX_HOOFS_POSIX_WRAPPER_SIGNAL_WATCHER_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SIGNAL_WATCHER_HPP

#include "iox/atomic.hpp"
#include "iox/optional.hpp"
#include "iox/signal_handler.hpp"
#include "iox/unnamed_semaphore.hpp"

namespace iox
{
/// @brief The SignalWatcher waits for SIGINT and SIGTERM. One can wait until the
///        signal has occurred or ask the watcher if it has occurred.
/// @code
///   // can be used to loop until SIGINT or SIGTERM has occurred
///   #include <iox/signal_watcher.hpp>
///   void loopUntilTerminationRequested()
///   {
///       while(!iox::hasTerminationRequested())
///       {
///           // your algorithm
///       }
///   }
///
///   // another possibility is to block until SIGINT or SIGTERM has occurred
///   void blockUntilCtrlC() {
///       // your objects which spawn threads
///       iox::waitForTerminationRequest();
///   }
/// @endcode
class SignalWatcher
{
  public:
    SignalWatcher(const SignalWatcher&) = delete;
    SignalWatcher(SignalWatcher&&) = delete;
    ~SignalWatcher() = default;

    SignalWatcher& operator=(const SignalWatcher&) = delete;
    SignalWatcher& operator=(SignalWatcher&&) = delete;

    /// @brief Returns the singleton instance of the SignalWatcher
    static SignalWatcher& getInstance() noexcept;

    /// @brief Blocks until either SIGTERM or SIGINT has occurred
    void waitForSignal() const noexcept;

    /// @brief Returns true when SIGTERM or SIGINT has occurred, otherwise false
    bool wasSignalTriggered() const noexcept;

  protected:
    SignalWatcher() noexcept;

  private:
    friend void internalSignalHandler(int) noexcept;
    mutable concurrent::Atomic<uint64_t> m_numberOfWaiters{0U};
    mutable optional<UnnamedSemaphore> m_semaphore;

    concurrent::Atomic<bool> m_hasSignalOccurred{false};
    SignalGuard m_sigTermGuard;
    SignalGuard m_sigIntGuard;
};

/// @brief convenience function, calls SignalWatcher::getInstance().waitForSignal();
void waitForTerminationRequest() noexcept;

/// @brief convenience function, calls SignalWatcher::getInstance().wasSignalTriggered();
bool hasTerminationRequested() noexcept;
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_SIGNAL_WATCHER_HPP
