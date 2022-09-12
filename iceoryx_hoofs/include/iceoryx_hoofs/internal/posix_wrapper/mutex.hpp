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
#ifndef IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP
#define IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_platform/pthread.hpp"

#if defined(__QNX__) || defined(__APPLE__)
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_FAST_NP PTHREAD_MUTEX_NORMAL
#endif

namespace iox
{
namespace posix
{
/// @brief Wrapper for a interprocess pthread based mutex which does not use
///         exceptions!
/// @code
///     #include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
///
///     int main() {
///         posix::mutex myMutex(false);
///
///         myMutex->lock();
///         // ... do stuff
///         myMutex->unlock();
///
///         {
///             std::lock_guard<posix::mutex> lock(myMutex);
///             // ...
///         }
///
///     }
/// @endcode
/// @attention Errors in c'tor or d'tor can lead to a program termination!
///
class mutex
{
  public:
    /// @attention the construction of the mutex can fail. This can lead to a program termination!
    explicit mutex(const bool f_isRecursive) noexcept;

    /// @attention the destruction of the mutex can fail. This can lead to a program termination!
    ~mutex() noexcept;

    /// @brief all copy and move assignment methods need to be deleted otherwise
    ///         undefined behavior or race conditions will occure if you copy
    ///         or move mutexe when its possible that they are locked or will
    ///         be locked
    mutex(const mutex&) = delete;
    mutex(mutex&&) = delete;
    mutex& operator=(const mutex&) = delete;
    mutex& operator=(mutex&&) = delete;

    /// @brief Locks the mutex object and returns true if the underlying c
    ///         function did not returned any error. If the mutex is already
    ///         locked the method is blocking till the mutex can be locked.
    bool lock() noexcept;
    /// @brief Unlocks the mutex object and returns true if the underlying c
    ///         function did not returned any error.
    ///        IMPORTANT! Unlocking and unlocked mutex is undefined behavior
    ///         and the underlying c function will report success in this case!
    bool unlock() noexcept;

    /// @brief  Tries to lock the mutex object. If it is not possible to lock
    ///         the mutex object try_lock will return an error. If the c
    ///         function fails it will return false, otherwise true.
    // NOLINTNEXTLINE(readability-identifier-naming) C++ STL code guidelines
    bool try_lock() noexcept;

    /// @brief  Returns the native handle which then can be used in
    ///         pthread_mutex_** calls. Required when a pthread_mutex_**
    ///         call is not abstracted with this wrapper.
    // NOLINTNEXTLINE(readability-identifier-naming) C++ STL code guidelines
    pthread_mutex_t get_native_handle() const noexcept;

    // NOLINTNEXTLINE(readability-identifier-naming) C++ STL code guidelines
    pthread_mutex_t m_handle{};
};
} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_MUTEX_HPP
