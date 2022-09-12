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
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_platform/platform_correction.hpp"

#include <cassert>
#include <utility>

namespace iox
{
namespace posix
{
mutex::mutex(bool f_isRecursive) noexcept
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

mutex::~mutex() noexcept
{
    auto destroyCall = posixCall(pthread_mutex_destroy)(&m_handle).returnValueMatchesErrno().evaluate();

    /// NOLINTJUSTIFICATION is fixed in the PR iox-#1443
    /// NOLINTNEXTLINE(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    cxx::Ensures(!destroyCall.has_error() && "Could not destroy mutex");
}

// NOLINTNEXTLINE(readability-identifier-naming) C++ STL code guidelines
pthread_mutex_t mutex::get_native_handle() const noexcept
{
    return m_handle;
}

bool mutex::lock() noexcept
{
    return !posixCall(pthread_mutex_lock)(&m_handle).returnValueMatchesErrno().evaluate().has_error();
}

bool mutex::unlock() noexcept
{
    return !posixCall(pthread_mutex_unlock)(&m_handle).returnValueMatchesErrno().evaluate().has_error();
}

// NOLINTNEXTLINE(readability-identifier-naming) C++ STL code guidelines
bool mutex::try_lock() noexcept
{
    auto result = posixCall(pthread_mutex_trylock)(&m_handle).returnValueMatchesErrno().ignoreErrnos(EBUSY).evaluate();
    bool isBusy = !result.has_error() && result->errnum == EBUSY;
    return !isBusy && !result.has_error();
}
} // namespace posix
} // namespace iox
