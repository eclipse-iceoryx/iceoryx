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
#ifndef IOX_HOOFS_POSIX_WRAPPER_SIGNAL_WATCHER_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SIGNAL_WATCHER_HPP

#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"
#include "iceoryx_hoofs/posix_wrapper/signal_handler.hpp"

#include <atomic>

namespace iox
{
namespace posix
{
class SignalWatcher
{
  public:
    SignalWatcher(const SignalWatcher&) = delete;
    SignalWatcher(SignalWatcher&&) = delete;
    ~SignalWatcher() = default;

    SignalWatcher& operator=(const SignalWatcher&) = delete;
    SignalWatcher& operator=(SignalWatcher&&) = delete;

    static SignalWatcher& getInstance() noexcept;

    void waitForSignal() const noexcept;

    bool wasSignalTriggered() const noexcept;


  private:
    SignalWatcher() noexcept;

  private:
    friend void internalSignalHandler(int) noexcept;
    mutable std::atomic<uint64_t> m_numberOfWaiters{0U};
    mutable Semaphore m_semaphore;

    std::atomic_bool m_hasSignalOccurred{false};
    SignalGuard m_sigTermGuard;
    SignalGuard m_sigIntGuard;
};
} // namespace posix
} // namespace iox

#endif
