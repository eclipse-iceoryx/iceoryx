// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/platform_correction.hpp"

#include "iceoryx_hoofs/platform/platform_correction.hpp"
namespace iox
{
namespace posix
{
cxx::expected<MutexError> MutexBuilder::create(cxx::optional<Mutex>& uninitializedMutex) noexcept
{
    pthread_mutexattr_t mutexAttributes;
    auto result = posixCall(pthread_mutexattr_init)(&mutexAttributes).returnValueMatchesErrno().evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case ENOMEM:
            LogError() << "Not enough memory to initialize required mutex attributes";
            return cxx::error<MutexError>(MutexError::INSUFFICIENT_MEMORY);
        default:
            LogError()
                << "This should never happen. An unknown error occurred while initializing the mutex attributes.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }


    cxx::GenericRAII mutexAttributesCleanup([&] {
        auto destroyResult =
            posixCall(pthread_mutexattr_destroy)(&mutexAttributes).returnValueMatchesErrno().evaluate();
        LogError() << "This should never happen. An unknown error occurred while cleaning up the mutex attributes.";
    });


    result = posixCall(pthread_mutexattr_setpshared)(
                 &mutexAttributes,
                 static_cast<int>((m_isInterProcessCapable) ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case ENOTSUP:
            LogError() << "The platform does not support shared mutex (inter process mutex)";
            return cxx::error<MutexError>(MutexError::INTER_PROCESS_MUTEX_UNSUPPORTED_BY_PLATFORM);
        default:
            LogError() << "This should never happen. An unknown error occurred while setting up the inter process "
                          "configuration.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }


    result = posixCall(pthread_mutexattr_settype)(&mutexAttributes, static_cast<int>(m_mutexType))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        LogError() << "This should never happen. An unknown error occurred while setting up the mutex type.";
        return cxx::error<MutexError>(MutexError::UNDEFINED);
    }


    result = posixCall(pthread_mutexattr_setprotocol)(&mutexAttributes, static_cast<int>(m_priorityInheritance))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        switch (result.get_error().errnum)
        {
        case ENOSYS:
            LogError() << "The system does not support mutex priorities\n";
            return cxx::error<MutexError>(MutexError::PRIORITIES_UNSUPPORTED_BY_PLATFORM);
        case ENOTSUP:
            LogError() << "The used mutex priority is not supported by the platform\n";
            return cxx::error<MutexError>(MutexError::USED_PRIORITY_UNSUPPORTED_BY_PLATFORM);
        case EPERM:
            LogError() << "Unsufficient permissions to set mutex priorities\n";
            return cxx::error<MutexError>(MutexError::PERMISSION_DENIED);
        default:
            LogError() << "This should never happen. An unknown error occurred while setting up the mutex priority.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }


    result = posixCall(pthread_mutexattr_setrobust)(&mutexAttributes, static_cast<int>(m_threadTerminationBehavior))
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        LogError() << "This should never happen. An unknown error occurred while setting up the mutex thread "
                      "termination behavior.";
        return cxx::error<MutexError>(MutexError::UNDEFINED);
    }


    uninitializedMutex.emplace();
    result = posixCall(pthread_mutex_init)(&uninitializedMutex->m_handle, &mutexAttributes)
                 .returnValueMatchesErrno()
                 .evaluate();
    if (result.has_error())
    {
        uninitializedMutex->m_isDescructable = false;
        uninitializedMutex.reset();

        switch (result.get_error().errnum)
        {
        case EAGAIN:
            LogError() << "Not enough resources to initialize another mutex.";
            return cxx::error<MutexError>(MutexError::INSUFFICIENT_RESOURCES);
        case ENOMEM:
            LogError() << "Not enough memory to initialize mutex.";
            return cxx::error<MutexError>(MutexError::INSUFFICIENT_MEMORY);
        case EPERM:
            LogError() << "Unsufficient permissions to create mutex.";
            return cxx::error<MutexError>(MutexError::PERMISSION_DENIED);
        default:
            LogError() << "This should never happen. An unknown error occurred while initializing the mutex handle. "
                          "This is possible when the handle is an already initialized mutex handle.";
            return cxx::error<MutexError>(MutexError::UNDEFINED);
        }
    }

    uninitializedMutex->m_isDescructable = true;
    return cxx::success<>();
}

Mutex::Mutex(bool f_isRecursive) noexcept
{
    pthread_mutexattr_t attr;
    bool isInitialized{true};
    isInitialized &= !posixCall(pthread_mutexattr_init)(&attr).returnValueMatchesErrno().evaluate().has_error();
    isInitialized &= !posixCall(pthread_mutexattr_setpshared)(&attr, PTHREAD_PROCESS_SHARED)
                          .returnValueMatchesErrno()
                          .evaluate()
                          .has_error();
    isInitialized &=
        !posixCall(pthread_mutexattr_settype)(&attr, f_isRecursive ? PTHREAD_MUTEX_RECURSIVE_NP : PTHREAD_MUTEX_FAST_NP)
             .returnValueMatchesErrno()
             .evaluate()
             .has_error();
    isInitialized &= !posixCall(pthread_mutexattr_setprotocol)(&attr, PTHREAD_PRIO_NONE)
                          .returnValueMatchesErrno()
                          .evaluate()
                          .has_error();
    isInitialized &= !posixCall(pthread_mutex_init)(&m_handle, &attr).returnValueMatchesErrno().evaluate().has_error();
    isInitialized &= !posixCall(pthread_mutexattr_destroy)(&attr).returnValueMatchesErrno().evaluate().has_error();

    /// NOLINTJUSTIFICATION is fixed in the PR iox-#1443
    /// NOLINTNEXTLINE(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    cxx::Ensures(isInitialized && "Unable to create mutex");
}

Mutex::~Mutex() noexcept
{
    if (m_isDescructable)
    {
        auto destroyCall = posixCall(pthread_mutex_destroy)(&m_handle).returnValueMatchesErrno().evaluate();

        /// NOLINTJUSTIFICATION is fixed in the PR iox-#1443
        /// NOLINTNEXTLINE(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        cxx::Ensures(!destroyCall.has_error() && "Could not destroy mutex");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming) C++ STL code guidelines
pthread_mutex_t Mutex::get_native_handle() const noexcept
{
    return m_handle;
}

bool Mutex::lock() noexcept
{
    return !posixCall(pthread_mutex_lock)(&m_handle).returnValueMatchesErrno().evaluate().has_error();
}

bool Mutex::unlock() noexcept
{
    return !posixCall(pthread_mutex_unlock)(&m_handle).returnValueMatchesErrno().evaluate().has_error();
}

// NOLINTNEXTLINE(readability-identifier-naming) C++ STL code guidelines
bool Mutex::try_lock() noexcept
{
    auto result = posixCall(pthread_mutex_trylock)(&m_handle).returnValueMatchesErrno().ignoreErrnos(EBUSY).evaluate();
    bool isBusy = !result.has_error() && result->errnum == EBUSY;
    return !isBusy && !result.has_error();
}
} // namespace posix
} // namespace iox
