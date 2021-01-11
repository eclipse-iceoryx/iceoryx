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

#ifndef IOX_UTILS_CONCURRENT_PERIODIC_TASK_HPP
#define IOX_UTILS_CONCURRENT_PERIODIC_TASK_HPP

#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "iceoryx_utils/posix_wrapper/thread.hpp"

#include <thread>

namespace iox
{
namespace concurrent
{
/// @brief This class periodically executes a callable specified by the template parameter.
///        This can be a struct with a `operator()()` overload, a `cxx::function_ref<void()>` or `std::fuction<void()>`
/// @tparam T is a callable type without function parameters
template <typename T>
class PeriodicTask
{
  public:
    /// @brief Creates a periodic task by spawning a thread. The specified callable is executed immediately on creation
    /// and then periodically after the interval duration.
    /// @param[in] taskName will be set as thread name
    /// @param[in] interval is the time the thread waits between two invocations of the callable
    /// @param[in] args are forwarded to the underlying callable object
    template <typename... Args>
    PeriodicTask(const posix::ThreadName_t taskName, const units::Duration interval, Args&&... args) noexcept;

    /// @brief Stops and joins the thread spawned by the constructor.
    /// @note This is blocking and the blocking time depends on the callable.
    ~PeriodicTask() noexcept;

    PeriodicTask(const PeriodicTask&) = delete;
    PeriodicTask(PeriodicTask&&) = delete;

    PeriodicTask& operator=(const PeriodicTask&) = delete;
    PeriodicTask& operator=(PeriodicTask&&) = delete;

  private:
    void run() noexcept;

  private:
    T m_callable;
    units::Duration m_interval;
    /// @todo use a refactored posix::Timer object once available
    posix::Semaphore m_stop{posix::Semaphore::create(posix::CreateUnnamedSingleProcessSemaphore, 0U).value()};
    std::thread m_taskExecutor{&PeriodicTask::run, this};
};

} // namespace concurrent
} // namespace iox

#include "iceoryx_utils/internal/concurrent/periodic_task.inl"

#endif // IOX_UTILS_CONCURRENT_PERIODIC_TASK_HPP
