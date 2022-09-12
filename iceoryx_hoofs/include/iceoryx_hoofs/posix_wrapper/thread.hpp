// Copyright (c) 2020, 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
#define IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/design_pattern/builder.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/pthread.hpp"

#include <atomic>

namespace iox
{
namespace posix
{
constexpr uint64_t MAX_THREAD_NAME_LENGTH = 15U;

using ThreadName_t = cxx::string<MAX_THREAD_NAME_LENGTH>;

/// @todo remove free functions
void setThreadName(iox_pthread_t thread, const ThreadName_t& name) noexcept;
ThreadName_t getThreadName(iox_pthread_t thread) noexcept;

enum class ThreadError
{
    EMPTY_CALLABLE,
    INSUFFICIENT_MEMORY,
    INSUFFICIENT_PERMISSIONS,
    INSUFFICIENT_RESOURCES,
    INVALID_ATTRIBUTES,
    UNDEFINED
};

/// @brief POSIX thread wrapper class. Following RAII, the thread is joined on destruction.
/// @code
/// #include "iceoryx_hoofs/posix_wrapper/thread.hpp"
///
/// iox::cxx::function<void()> callable = []() { /* ... */ };
/// optional<Thread> myThread;
/// ThreadBuilder().create(myThread, callable).expect("Couldn't create a thread.");
/// @endcode
/// NOLINTJUSTIFICATION m_threadHandle is always initialized during create in the ThreadBuilder hence it is impossible
/// to create a Thread without an initialized m_threadHandle
/// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
class Thread
{
  public:
    using callable_t = cxx::function<void()>;

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;

    ~Thread() noexcept;

    /// @brief Returns the name of the thread
    /// @return An iox::cxx::string containing the name of the thread
    ThreadName_t getName() const noexcept;

    friend class ThreadBuilder;
    friend class cxx::optional<Thread>;

  private:
    Thread() noexcept = default;

    static ThreadError errnoToEnum(const int errnoValue) noexcept;

    static void* startRoutine(void* callable);

    iox_pthread_t m_threadHandle;
    callable_t m_callable;
    bool m_isThreadConstructed{false};
    ThreadName_t m_threadName;
};

class ThreadBuilder
{
    /// @brief Set the name of the thread
    IOX_BUILDER_PARAMETER(ThreadName_t, name, "")

  public:
    /// @brief Creates a thread
    /// @param[in] uninitializedThread is an iox::cxx::optional where the thread is stored
    /// @param[in] callable is the callable that is invoked by the thread
    /// @return an error describing the failure or success
    cxx::expected<ThreadError> create(cxx::optional<Thread>& uninitializedThread,
                                      const Thread::callable_t& callable) noexcept;
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
