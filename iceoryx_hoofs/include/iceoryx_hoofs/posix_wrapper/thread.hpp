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
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/platform/pthread.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <atomic>

namespace iox
{
namespace posix
{
constexpr uint64_t MAX_THREAD_NAME_LENGTH = 15U;

using ThreadName_t = cxx::string<MAX_THREAD_NAME_LENGTH>;

// make those methods of thread class
void setThreadName(pthread_t thread, const ThreadName_t& name) noexcept;
ThreadName_t getThreadName(pthread_t thread) noexcept;

enum class ThreadError
{
    EMPTY_CALLABLE,
    INSUFFICIENT_MEMORY,
    INSUFFICIENT_PERMISSIONS,
    INSUFFICIENT_RESOURCES,
    INVALID_ATTRIBUTES,
    UNDEFINED
};

class ThreadBuilder;

class thread
{
  public:
    using callable_t = cxx::function<void(), sizeof(cxx::function<void()>) + alignof(cxx::function<void()>)>;

    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;
    thread(thread&& other) = delete;
    thread& operator=(thread&& rhs) = delete;

    ~thread() noexcept
    {
        if (m_destroy)
        {
            if (m_isJoinable)
            {
                /// @todo replace nullptr?
                auto joinResult = posixCall(pthread_join)(m_threadHandle, nullptr).successReturnValue(0).evaluate();
                if (joinResult.has_error())
                {
                    switch (joinResult.get_error().errnum)
                    {
                    case EDEADLK:
                        LogError() << "A deadlock was detected when attempting to join the thread.";
                        break;
                    default:
                        LogError() << "This should never happen. An unknown error occurred.";
                        break;
                    }
                }
                m_isJoinable = false;
            }
        }
    }

    friend class ThreadBuilder;
    friend class cxx::optional<thread>;

  private:
    thread() noexcept = default;

    static ThreadError errnoToEnum(const int errnoValue) noexcept
    {
        switch (errnoValue)
        {
        case EAGAIN:
            LogError() << "insufficient resources to create another thread";
            return ThreadError::INSUFFICIENT_RESOURCES;
        case EINVAL:
            LogError() << "invalid attribute settings";
            return ThreadError::INVALID_ATTRIBUTES;
        case ENOMEM:
            LogError() << "not enough memory to initialize the thread attributes object";
            return ThreadError::INSUFFICIENT_MEMORY;
        case EPERM:
            LogError() << "no appropriate permission to set required scheduling policy or parameters";
            return ThreadError::INSUFFICIENT_PERMISSIONS;
        default:
            LogError() << "an unexpected error occurred in thread - this should never happen! errno: "
                       << strerror(errnoValue);
            return ThreadError::UNDEFINED;
        }
    }

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
            LogError() << "The thread cannot be created with an empty callable.";
            return cxx::error<ThreadError>(ThreadError::EMPTY_CALLABLE);
        }

        uninitializedThread.emplace();
        uninitializedThread->m_callable = [=] { callable(std::forward<CallableArgs>(args)...); };

        // set attributes
        pthread_attr_t attr;
        auto initResult = posixCall(pthread_attr_init)(&attr).successReturnValue(0).evaluate();
        if (initResult.has_error())
        {
            uninitializedThread->m_destroy = false; /// @todo replace with m_isJoinable?
            uninitializedThread.reset();
            return cxx::error<ThreadError>(thread::errnoToEnum(initResult.get_error().errnum));
        }

        auto setDetachStateResult = posixCall(pthread_attr_setdetachstate)(
                                        &attr, m_detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE)
                                        .successReturnValue(0)
                                        .evaluate();
        if (setDetachStateResult.has_error())
        {
            uninitializedThread->m_destroy = false; /// @todo replace with m_isJoinable?
            uninitializedThread.reset();
            LogError() << "Something went wrong when setting the detach state. This should never happen!";
            return cxx::error<ThreadError>(thread::errnoToEnum(setDetachStateResult.get_error().errnum));
        }
        uninitializedThread->m_isJoinable = !m_detached;

        // create thread
        auto createResult =
            posixCall(pthread_create)(
                &uninitializedThread->m_threadHandle, &attr, thread::cbk, &uninitializedThread->m_callable)
                .successReturnValue(0)
                .evaluate();
        posixCall(pthread_attr_destroy)(&attr).successReturnValue(0).evaluate().or_else([](auto&) {
            LogError() << "Something went wrong when destroying the thread attributes object.";
        }); /// @todo not clear if pthread_attr_destroy can fail, specifications differ. Do we have to care if it fails?
        if (createResult.has_error())
        {
            uninitializedThread->m_destroy = false; /// @todo replace with m_isJoinable?
            uninitializedThread.reset();
            return cxx::error<ThreadError>(thread::errnoToEnum(createResult.get_error().errnum));
        }

        return cxx::success<>();
    }
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_PTHREAD_HPP
