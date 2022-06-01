// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_hoofs/platform/pthread.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <atomic>

namespace iox
{
namespace posix
{
constexpr uint64_t MAX_THREAD_NAME_LENGTH = 15U;

using ThreadName_t = cxx::string<MAX_THREAD_NAME_LENGTH>;

enum class ThreadError
{
    EMPTY_CALLABLE,
    INSUFFICIENT_MEMORY,
    INSUFFICIENT_PERMISSIONS,
    INSUFFICIENT_RESOURCES,
    INVALID_ATTRIBUTES,
    UNDEFINED
};

void setThreadName(pthread_t thread, const ThreadName_t& name) noexcept;
ThreadName_t getThreadName(pthread_t thread) noexcept;

class thread
{
  public:
    using callable_t = cxx::function<void(), sizeof(cxx::function<void()>) + alignof(cxx::function<void()>)>;

    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;
    thread(thread&& other) = delete;
    thread& operator=(thread&& rhs) = delete;

    ~thread() noexcept;

    void setThreadName(const ThreadName_t& name) noexcept;

    ThreadName_t getThreadName() noexcept;

    friend class ThreadBuilder;
    friend class cxx::optional<thread>;

  private:
    thread() noexcept = default;

    static ThreadError errnoToEnum(const int errnoValue) noexcept;

    static void* cbk(void* callable);

    pthread_t m_threadHandle;
    callable_t m_callable;
    bool m_isJoinable{true};
    bool m_destroy{true};
};

class ThreadBuilder
{
    /// @todo set m_isJoinable directly?
    IOX_BUILDER_PARAMETER(bool, detached, false)

  public:
    template <typename Signature, typename... CallableArgs>
    cxx::expected<ThreadError> create(cxx::optional<thread>& uninitializedThread,
                                      const cxx::function<Signature>& callable,
                                      CallableArgs&&... args) noexcept;
};

} // namespace posix
} // namespace iox

#include "iceoryx_hoofs/internal/posix_wrapper/thread.inl"

#endif // IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
