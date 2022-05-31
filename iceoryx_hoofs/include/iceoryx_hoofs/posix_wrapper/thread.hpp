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

/// @todo add possible error codes
enum class ThreadError
{
    UNKNOWN_ERROR
};

class ThreadBuilder;

class thread
{
  public:
    using callable_t = cxx::function<void(), sizeof(cxx::function<void()>) + alignof(cxx::function<void()>)>;

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
        if (m_destroy)
        {
            if (m_isJoinable)
            {
                /// @todo replace nullptr?, check errors
                posixCall(pthread_join)(m_threadHandle, nullptr).successReturnValue(0).evaluate();
                m_isJoinable = false;
            }
        }
    }

    friend class ThreadBuilder;
    friend class cxx::optional<thread>;

  private:
    thread() noexcept = default;

    static void* cbk(void* callable)
    {
        // necessary because the callback signature for pthread_create is void*(void*)
        callable_t f = *static_cast<callable_t*>(callable);
        f();
        return nullptr;
    }

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
                                      CallableArgs&&... args) noexcept
    {
        if (!callable)
        {
            return cxx::error<ThreadError>();
        }

        uninitializedThread.emplace();
        uninitializedThread.value().m_callable = [=] { callable(std::forward<CallableArgs>(args)...); };

        // set attributes
        pthread_attr_t attr;
        auto initResult = posixCall(pthread_attr_init)(&attr).successReturnValue(0).evaluate();
        if (initResult.has_error())
        {
            uninitializedThread.value().m_destroy = false; /// @todo replace with m_isJoinable?
            uninitializedThread.reset();
            return cxx::error<ThreadError>();
        }

        auto detachState = posixCall(pthread_attr_setdetachstate)(
                               &attr, m_detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE)
                               .successReturnValue(0)
                               .evaluate();
        /// @todo Will only fail when invalid value was passed which is not the case. Remove error check?
        if (detachState.has_error())
        {
            uninitializedThread.value().m_destroy = false; /// @todo replace with m_isJoinable?
            uninitializedThread.reset();
            return cxx::error<ThreadError>();
        }
        uninitializedThread->m_isJoinable = !m_detached;

        auto result = posixCall(pthread_create)(&uninitializedThread.value().m_threadHandle,
                                                &attr,
                                                thread::cbk,
                                                &uninitializedThread.value().m_callable)
                          .successReturnValue(0)
                          .evaluate();
        posixCall(pthread_attr_destroy)(&attr); /// @todo could fail but should we care?
        if (result.has_error())
        {
            uninitializedThread.value().m_destroy = false; /// @todo replace with m_isJoinable?
            uninitializedThread.reset();
            return cxx::error<ThreadError>();
        }

        return cxx::success<>();
    }
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
