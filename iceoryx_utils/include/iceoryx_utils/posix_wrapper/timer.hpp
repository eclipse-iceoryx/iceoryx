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

#include "iceoryx_utils/design_pattern/creation.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

#include <cstdint>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

namespace iox
{
namespace posix
{
enum class TimerError
{
    NO_ERROR,
    TIMER_NOT_INITIALIZED,
    NO_VALID_CALLBACK,
    KERNEL_ALLOC_FAILED,
    INVALID_ARGUMENTS,
    ALLOC_MEM_FAILED,
    NO_PERMISSION,
    INVALID_POINTER,
    NO_TIMER_TO_DELETE,
    INTERNAL_LOGIC_ERROR
};

using namespace iox::units::duration_literals;

/// @brief Interface for timers on POSIX operating systems
/// @note Can't be copied or moved as operating system has a pointer to this object. It needs to be ensured that this
/// object lives longer than timeToWait, otherwise the operating system will unregister the timer
///
/// @code
///     posix::Timer TiborTheTimer(100_ms, [&]() { fooBar++; });
///
///     // Start a periodic timer
///     TiborTheTimer.start(true);
///     // [.. wait ..]
///     // Timer fires after 100_ms and calls the lambda which increments fooBar
///
///     TiborTheTimer.stop();
///
/// @endcode
class Timer
{
  public:
#ifdef __QNX__
    static constexpr timer_t INVALID_TIMER_ID = 0;
#else
    static constexpr timer_t INVALID_TIMER_ID = nullptr;
#endif

    /// @brief creates Duration from the result of clock_gettime(CLOCK_REALTIME, ...)
    /// @return if the clock_gettime call failed TimerError is returned otherwise Duration
    /// @todo maybe move this to a clock implementation?
    static cxx::expected<units::Duration, TimerError> now() noexcept;

    /// @brief Creates a timer without an operating system callback
    ///
    /// Creates a light-weight timer object that can be used with 
    ///               * hasExpiredComparedToCreationTime()
    ///               * resetCreationTime()
    ///
    /// @param[in] timeToWait - How long should be waited?
    /// @note Does not set up an operating system timer, but uses CLOCK_REALTIME instead
    Timer(units::Duration timeToWait) noexcept;

    /// @brief Creates a timer with an operating system callback
    ///
    /// Initially the timer is stopped.
    ///
    /// @param[in] timeToWait - How long should be waited?
    /// @note Operating systems needs a valid reference to this object, hence DesignPattern::Creation can't be used
    Timer(units::Duration timeToWait, std::function<void()> callback) noexcept;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer(const Timer& other) = delete;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer(Timer&& other) = delete;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer& operator=(const Timer& other) = delete;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer& operator=(Timer&& other) = delete;

    /// @brief D'tor
    virtual ~Timer() noexcept;

    /// @brief Starts the timer
    ///
    /// The callback is called by the operating system after the time has expired.
    ///
    /// @param[in] periodic - can be a periodic timer if set to true, default false
    /// @note Shall only be called when callback is given
    cxx::expected<TimerError> start(bool periodic = false) noexcept;

    /// @brief Disarms the timer
    /// @note Shall only be called when callback is given
    cxx::expected<TimerError> stop() noexcept;

    /// @brief Disarms the timer, assigns a new timeToWait value and arms the timer
    /// @note Shall only be called when callback is given
    cxx::expected<TimerError> restart(units::Duration timeToWait, bool periodic = false) noexcept;

    /// @brief Resets the internal creation time
    void resetCreationTime() noexcept;

    /// @brief Checks if the timer has expired compared to its creation time
    /// @return Is the elapsed time larger than timeToWait?
    bool hasExpiredComparedToCreationTime() noexcept;

    // @brief Returns the time until the timer expires the next time
    /// @note Shall only be called when callback is given
    cxx::expected<units::Duration, TimerError> timeUntilExpiration() noexcept;

    /// @brief In case the callback is not immediately called by the operating system, getOverruns() returns the
    /// additional overruns that happended in the delay interval
    /// @note Shall only be called when callback is given
    cxx::expected<uint64_t, TimerError> getOverruns() noexcept;

    /// @brief Call the user-defined callback
    /// @note This call is wrapped in a plain C function
    void executeCallback() noexcept;

    /// @brief Returns true if the construction of the object was successful
    bool hasError() const noexcept;

    /// @brief Returns the error that occured on constructing the object
    cxx::error<TimerError> getError() const noexcept;

  private:
    /// @brief Remove the timer from the operating system
    cxx::expected<TimerError> deleteTimer() noexcept;

    /// @brief Converts errnum to TimerError
    static cxx::error<TimerError> createErrorFromErrno(const int errnum) noexcept;

    /// @brief Duration after the timer calls the user-defined callback function
    units::Duration m_timeToWait;

    /// @brief Time when the timer object was created
    units::Duration m_creationTime;

    /// @brief Identifier for the timer in the operating system
    timer_t m_timerId;

    /// @brief Stores the user-defined callback
    std::function<void()> m_callback;

    /// @brief True if start() was called, set to false when stop() is called
    bool m_armed{false};

    /// @brief Bool that signals whether the object is fully initalized
    bool m_isInitialized{false};

    /// @brief If an error happened during creation the value is stored in here
    TimerError m_errorValue{TimerError::NO_ERROR};
};

} // namespace posix
} // namespace iox
