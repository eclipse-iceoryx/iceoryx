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
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace posix
{
void setThreadName(iox_pthread_t thread, const ThreadName_t& name) noexcept
{
    posixCall(iox_pthread_setname_np)(thread, name.c_str()).successReturnValue(0).evaluate().or_else([](auto& r) {
        // String length limit is ensured through cxx::string
        // ERANGE (string too long) intentionally not handled to avoid untestable and dead code
        LogError() << "This should never happen! " << r.getHumanReadableErrnum();
        cxx::Ensures(false && "internal logic error");
    });
}

ThreadName_t getThreadName(iox_pthread_t thread) noexcept
{
    // NOLINTJUSTIFICATION required as name buffer for iox_pthread_getname_np
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char tempName[MAX_THREAD_NAME_LENGTH + 1U];

    posixCall(iox_pthread_getname_np)(thread, &tempName[0], MAX_THREAD_NAME_LENGTH + 1U)
        .successReturnValue(0)
        .evaluate()
        .or_else([](auto& r) {
            // String length limit is ensured through MAX_THREAD_NAME_LENGTH
            // ERANGE (string too small) intentionally not handled to avoid untestable and dead code
            LogError() << "This should never happen! " << r.getHumanReadableErrnum();
            cxx::Ensures(false && "internal logic error");
        });

    return ThreadName_t(cxx::TruncateToCapacity, &tempName[0]);
}

cxx::expected<ThreadError> ThreadBuilder::create(cxx::optional<Thread>& uninitializedThread,
                                                 const Thread::callable_t& callable) noexcept
{
    if (!callable)
    {
        LogError() << "The thread cannot be created with an empty callable.";
        return cxx::error<ThreadError>(ThreadError::EMPTY_CALLABLE);
    }

    uninitializedThread.emplace();
    uninitializedThread->m_callable = callable;

    uninitializedThread->m_threadName = m_name;

    const iox_pthread_attr_t* threadAttributes = nullptr;

    auto createResult =
        posixCall(iox_pthread_create)(
            &uninitializedThread->m_threadHandle, threadAttributes, Thread::startRoutine, &uninitializedThread.value())
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
        auto joinResult = posixCall(iox_pthread_join)(m_threadHandle, nullptr).successReturnValue(0).evaluate();
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

ThreadName_t Thread::getName() const noexcept
{
    return m_threadName;
}

ThreadError Thread::errnoToEnum(const int errnoValue) noexcept
{
    switch (errnoValue)
    {
    case EAGAIN:
        /// @todo iox-#1365 add thread name to log message once the name is set via BUILDER_PARAMETER, maybe add both,
        /// the name of the new thread and the name of the thread which created the new one
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
        LogError() << "an unexpected error occurred in thread - this should never happen!";
        return ThreadError::UNDEFINED;
    }
}

void* Thread::startRoutine(void* callable)
{
    auto* self = static_cast<Thread*>(callable);
    auto threadHandle = iox_pthread_self();

    posixCall(iox_pthread_setname_np)(threadHandle, self->m_threadName.c_str())
        .successReturnValue(0)
        .evaluate()
        .or_else([&self](auto&) {
            LogWarn() << "failed to set thread name " << self->m_threadName;
            self->m_threadName.clear();
        });

    self->m_callable();
    return nullptr;
}
} // namespace posix
} // namespace iox
