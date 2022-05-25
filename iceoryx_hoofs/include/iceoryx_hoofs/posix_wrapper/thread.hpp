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

void setThreadName(pthread_t thread, const ThreadName_t& name) noexcept;
ThreadName_t getThreadName(pthread_t thread) noexcept;

// add possible error codes
enum class ThreadError
{
    UNKNOWN_ERROR
};

class ThreadBuilder;

class thread
{
  public:
    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    thread(thread&& other) noexcept
    {
        *this = std::move(other);
    }

    thread& operator=(thread&& rhs) noexcept
    {
        if (this != &rhs)
        {
            m_threadHandle = rhs.m_threadHandle;
            m_isJoinable = rhs.m_isJoinable;
            rhs.m_isJoinable = false;
        }
        return *this;
    }

    ~thread() noexcept
    {
        if (m_isJoinable)
        {
            // replace nullptr?, check errors
            posixCall(pthread_join)(m_threadHandle, nullptr).successReturnValue(0).evaluate();
        }
    }

    friend class ThreadBuilder;

  private:
    thread(const pthread_t handle) noexcept
        : m_threadHandle(handle)
    {
    }

    pthread_t m_threadHandle;
    bool m_isJoinable = true;
};

inline void* cbk(void* callable)
{
    // Necessary because the callback signature for pthread_create is void*(void*)
    cxx::function<void()> f = *static_cast<cxx::function<void()>*>(callable);
    f();
    return nullptr;
}

class ThreadBuilder
{
  public:
    template <typename Callable, typename... CallableArgs>
    cxx::expected<thread, ThreadError> create(const Callable& callable, CallableArgs&&... args) noexcept
    {
        pthread_t handle;
        std::atomic_bool callableHasStarted{false};
        cxx::function<void()> f([=, &callableHasStarted] {
            callableHasStarted.store(true);
            callable(std::forward<CallableArgs>(args)...);
        });
        auto result = posixCall(pthread_create)(&handle, nullptr, cbk, &f).successReturnValue(0).evaluate();
        if (result)
        {
            // Needed, otherwise thread might be created without executing the callback yet when we return. Then the
            // callable and its arguments are not known anymore.
            while (callableHasStarted.load() == false)
            {
            }
            return cxx::success<thread>(thread(handle));
        }
        return cxx::error<ThreadError>();
    }
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
