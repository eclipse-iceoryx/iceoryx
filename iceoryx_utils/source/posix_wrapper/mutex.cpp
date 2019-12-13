// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include <cassert>
#include <utility>

namespace iox
{
namespace posix
{
cxx::optional<mutex> mutex::CreateMutex(const bool f_isRecursive)
{
    cxx::optional<mutex> returnValue;
    returnValue.emplace(f_isRecursive);
    if (!returnValue->m_isInitialized)
    {
        return cxx::nullopt_t();
    }
    return returnValue;
}

mutex::mutex(bool f_isRecursive)
{
    pthread_mutexattr_t attr;
    m_isInitialized &=
        !cxx::makeSmartC(pthread_mutexattr_init, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, &attr).hasErrors();
    m_isInitialized &= !cxx::makeSmartC(pthread_mutexattr_setpshared,
                                        cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE,
                                        {0},
                                        {},
                                        &attr,
                                        PTHREAD_PROCESS_SHARED)
                            .hasErrors();
    m_isInitialized &= !cxx::makeSmartC(pthread_mutexattr_settype,
                                        cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE,
                                        {0},
                                        {},
                                        &attr,
                                        f_isRecursive ? PTHREAD_MUTEX_RECURSIVE_NP : PTHREAD_MUTEX_FAST_NP)
                            .hasErrors();
    m_isInitialized &= !cxx::makeSmartC(pthread_mutexattr_setprotocol,
                                        cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE,
                                        {0},
                                        {},
                                        &attr,
                                        PTHREAD_PRIO_NONE)
                            .hasErrors();

    m_isInitialized &=
        !cxx::makeSmartC(pthread_mutex_init, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, &m_handle, &attr)
             .hasErrors();
}

mutex::mutex(mutex&& rhs) noexcept
{
    m_handle = rhs.m_handle;
    m_isInitialized = rhs.m_isInitialized;
    rhs.m_isInitialized = false;
}

mutex::~mutex()
{
    if (m_isInitialized)
    {
        auto destroyCall =
            cxx::makeSmartC(pthread_mutex_destroy, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, &m_handle);

        if (destroyCall.hasErrors())
        {
            std::cerr << "could not destroy mutex ::: pthread_mutex_destroy returned " << destroyCall.getReturnValue()
                      << " "
                      << "( " << strerror(destroyCall.getReturnValue()) << ") " << std::endl;
            std::terminate();
        }
    }
}

pthread_mutex_t mutex::get_native_handle() const noexcept
{
    return m_handle;
}

bool mutex::lock()
{
    return !cxx::makeSmartC(pthread_mutex_lock, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, &m_handle)
                .hasErrors();
}

bool mutex::unlock()
{
    return !cxx::makeSmartC(pthread_mutex_unlock, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, &m_handle)
                .hasErrors();
}

bool mutex::try_lock()
{
    return !cxx::makeSmartC(pthread_mutex_trylock, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, &m_handle)
                .hasErrors();
}
} // namespace posix
} // namespace iox
