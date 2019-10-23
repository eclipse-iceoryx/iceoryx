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

#pragma once

#include <mutex>

namespace iox
{
namespace concurrent
{
/// @brief The smart_lock class is a wrapping class which can be used to make
///         an arbitrary class threadsafe by wrapping it with the help of the
///         arrow operator.
///         IMPORTANT: If you generate a threadsafe container with smart_lock,
///                     only the container is threadsafe not the containing
///                     elements!
/// @code
///     #include <algorithm>
///     #include <vector>
///     #include "smart_lock.hpp"
///
///     int main() {
///         concurrent::smart_lock<std::vector<int>> threadSafeVector;
///         threadSafeVector->push_back(123);
///         threadSafeVector->push_back(456);
///         threadSafeVector->push_back(789);
///         size_t vectorSize = threadSafeVector->size();
///
///         {
///             auto guardedVector = threadSafeVector.GetScopeGuard();
///             auto iter = std::find(guardVector->begin(), guardVector->end(), 456);
///             if (iter != guardVector->end()) guardVector->erase(iter);
///         }
///     }
/// @endcode
template <typename T, typename MutexType = std::mutex>
class smart_lock
{
  private:
    class Proxy
    {
      public:
        Proxy(T* base, MutexType* lock);
        ~Proxy();

        T* operator->();

      private:
        T* base;
        MutexType* lock;
    };

  public:
    smart_lock();
    smart_lock(const T& t);
    smart_lock(const smart_lock& rhs);
    smart_lock(smart_lock&& rhs);
    smart_lock& operator=(const smart_lock& rhs);
    smart_lock& operator=(smart_lock&& rhs);
    ~smart_lock();

    /// @brief The arrow operator returns a proxy object which locks the mutex
    ///         of smart_lock and has another arrow operator defined which
    ///         returns the pointer of the underlying object. You use this
    ///         operator to call an arbitrary method of the base object which
    ///         is secured by the smart_lock mutex
    /// @code
    ///     iox::concurrent::smart_lock<std::vector<int>> threadSafeVector;
    ///     threadSafeVector->push_back(123); // this call is secured by a mutex
    /// @endcode
    Proxy operator->();

    /// @brief If you need to lock your object over multiple method calls you
    ///         acquire a scope guard which locks the object as long as this
    ///         guard is in scope, like a std::lock_guard.
    ///
    ///         IMPORTANT:
    ///         You need to with this guard in that scope and not with the
    ///         smart_lock object, otherwise a deadlock occurs!
    /// @code
    ///     iox::concurrent::smart_lock<std::vector<int>> threadSafeVector;
    ///
    ///     // The following scope is secured by the smart_lock mutex. In that
    ///     // scope you should not use the -> operator of threadSafeVector
    ///     // since it would lead to a deadlock.
    ///     // You access the underlying object by using the vectorGuard object!
    ///     {
    ///         auto vectorGuard = threadSafeVector.GetScopeGuard();
    ///         auto iter = std::find(vectorGuard->begin(), vectorGuard->end(),
    ///                 123);
    ///         if ( iter != vectorGuard->end() )
    ///             vectorGuard->erase(iter);
    ///     }
    Proxy GetScopeGuard();

  private:
    T base;
    MutexType lock;
};
} // namespace concurrent
} // namespace iox

#include "iceoryx_utils/internal/concurrent/smart_lock.inl"
