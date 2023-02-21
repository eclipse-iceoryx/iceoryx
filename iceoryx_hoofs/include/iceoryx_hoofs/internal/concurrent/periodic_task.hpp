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

#ifndef IOX_HOOFS_CONCURRENT_PERIODIC_TASK_HPP
#define IOX_HOOFS_CONCURRENT_PERIODIC_TASK_HPP

#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "iceoryx_hoofs/posix_wrapper/unnamed_semaphore.hpp"
#include "iox/duration.hpp"
#include "iox/string.hpp"

#include <thread>

#include <iostream>

namespace iox
{
namespace concurrent
{
/// @brief This is a helper struct to make the immediate start of the task in the PeriodicTask ctor obvious to the user
struct PeriodicTaskAutoStart_t
{
};
static constexpr PeriodicTaskAutoStart_t PeriodicTaskAutoStart;

/// @brief This is a helper struct to make the manual start of the task with the 'start' method obvious to the user
struct PeriodicTaskManualStart_t
{
};
static constexpr PeriodicTaskManualStart_t PeriodicTaskManualStart;

/// @brief This class periodically executes a callable specified by the template parameter.
///        This can be a struct with a 'operator()()' overload, a 'iox::function_ref<void()>' or 'std::fuction<void()>'.
/// @code
/// #include <iceoryx_hoofs/internal/concurrent/periodic_task.hpp>
/// #include <iox/duration.hpp>
/// #include <iostream>
///
/// int main()
/// {
///     using namespace iox::units::duration_literals;
///     PeriodicTask<iox::function_ref<void()>> task{
///         PeriodicTaskAutoStart, 1_s, "MyTask", [] { IOX_LOG(INFO) << "Hello World"; }};
///
///         return 0;
/// }
/// @endcode
/// @note Currently execution time of the callable is added to the interval.
/// @tparam T is a callable type without function parameters
template <typename T>
class PeriodicTask
{
  public:
    /// @brief Creates a periodic task. The specified callable is stored but not executed.
    /// To run the task, 'void start(const units::Duration interval)' must be called.
    /// @tparam Args are variadic template parameter for which are forwarded to the underlying callable object
    /// @param[in] PeriodicTaskManualStart_t indicates that this ctor doesn't start the task; just pass
    /// 'PeriodicTaskManualStart' as argument
    /// @param[in] taskName will be set as thread name
    /// @param[in] args are forwarded to the underlying callable object
    template <typename... Args>
    // PeriodicTaskManualStart_t is a compile time constant to indicate that this constructor does not start the task
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    PeriodicTask(const PeriodicTaskManualStart_t, const posix::ThreadName_t& taskName, Args&&... args) noexcept;

    /// @brief Creates a periodic task by spawning a thread. The specified callable is executed immediately on creation
    /// and then periodically after the interval duration.
    /// @tparam Args are variadic template parameter for which are forwarded to the underlying callable object
    /// @param[in] PeriodicTaskAutoStart_t indicates that this ctor starts the task; just pass
    /// 'PeriodicTaskAutoStart' as argument
    /// @param[in] interval is the time the thread waits between two invocations of the callable
    /// @param[in] taskName will be set as thread name
    /// @param[in] args are forwarded to the underlying callable object
    template <typename... Args>
    // PeriodicTaskAutoStart_t is a compile time constant to indicate that this constructor starts the task
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    PeriodicTask(const PeriodicTaskAutoStart_t,
                 const units::Duration interval,
                 const posix::ThreadName_t& taskName,
                 Args&&... args) noexcept;

    /// @brief Stops and joins the thread spawned by the constructor.
    /// @note This is blocking and the blocking time depends on the callable.
    ~PeriodicTask() noexcept;

    PeriodicTask(const PeriodicTask&) = delete;
    PeriodicTask(PeriodicTask&&) = delete;

    PeriodicTask& operator=(const PeriodicTask&) = delete;
    PeriodicTask& operator=(PeriodicTask&&) = delete;

    /// @brief Spawns a thread and immediately executes the callable specified with the constructor.
    /// The execution is repeated after the specified interval is passed.
    /// @param[in] interval is the time the thread waits between two invocations of the callable
    /// @attention If the PeriodicTask instance has already a running thread, this will be stopped and started again
    /// with the new interval. This might take some time if a slow task is executing during this call.
    void start(const units::Duration interval) noexcept;

    /// @brief This stops the thread if it's running, otherwise does nothing. When this method returns, the thread is
    /// stopped.
    /// @attention This might take some time if a slow task is executing during this call.
    void stop() noexcept;

    /// @brief This method check if a thread is spawned and running, potentially executing a task.
    /// @return true if the thread is running, false otherwise.
    bool isActive() const noexcept;

  private:
    void run() noexcept;

  private:
    T m_callable;
    posix::ThreadName_t m_taskName;
    units::Duration m_interval{units::Duration::fromMilliseconds(0U)};
    optional<posix::UnnamedSemaphore> m_stop;
    std::thread m_taskExecutor;
};

} // namespace concurrent
} // namespace iox

#include "iceoryx_hoofs/internal/concurrent/periodic_task.inl"

#endif // IOX_HOOFS_CONCURRENT_PERIODIC_TASK_HPP
