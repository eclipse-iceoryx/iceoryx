// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace posix
{
cxx::expected<ThreadError> ThreadBuilder::create(cxx::optional<Thread>& uninitializedThread,
                                                 const Thread::callable_t& callable) noexcept
{
    if (!callable)
    {
        std::cerr << "The thread cannot be created with an empty callable." << std::endl;
        return cxx::error<ThreadError>(ThreadError::EMPTY_CALLABLE);
    }

    uninitializedThread.emplace();
    uninitializedThread->m_callable = callable;

    const pthread_attr_t* threadAttributes = nullptr;

    auto createResult = posixCall(pthread_create)(&uninitializedThread->m_threadHandle,
                                                  threadAttributes,
                                                  Thread::startRoutine,
                                                  &uninitializedThread->m_callable)
                            .successReturnValue(0)
                            .evaluate();
    uninitializedThread->m_isThreadConstructed = !createResult.has_error();
    if (!uninitializedThread->m_isThreadConstructed)
    {
        uninitializedThread.reset();
        return cxx::error<ThreadError>(Thread::errnoToEnum(createResult.get_error().errnum));
    }

    return cxx::success<>();
}

Thread::~Thread() noexcept
{
    if (m_isThreadConstructed)
    {
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
    }
}

void Thread::setName(const ThreadName_t& name) noexcept
{
    posixCall(iox_pthread_setname_np)(m_threadHandle, name.c_str())
        .successReturnValue(0)
        .evaluate()
        .expect("This should never happen! Failed to set thread name.");
    /// @todo thread specific comm file under /proc/self/task/[tid]/comm is read. Opening this file can fail
    /// and errors possible for open(2) can be retrieved. Handle them here?
}

ThreadName_t Thread::getName() noexcept
{
    char tempName[MAX_THREAD_NAME_LENGTH + 1U];

    posixCall(pthread_getname_np)(m_threadHandle, tempName, MAX_THREAD_NAME_LENGTH + 1U)
        .successReturnValue(0)
        .evaluate()
        .expect("This should never happen! Failed to retrieve the thread name.");
    /// @todo thread specific comm file under /proc/self/task/[tid]/comm is read. Opening this file can fail
    /// and errors possible for open(2) can be retrieved. Handle them here?

    return ThreadName_t(cxx::TruncateToCapacity, tempName);
}

ThreadError Thread::errnoToEnum(const int errnoValue) noexcept
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

void* Thread::startRoutine(void* callable)
{
    (*static_cast<callable_t*>(callable))();
    return nullptr;
}
} // namespace posix
} // namespace iox
