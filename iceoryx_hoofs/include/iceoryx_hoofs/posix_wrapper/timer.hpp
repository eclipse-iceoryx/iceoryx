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
#ifndef IOX_HOOFS_POSIX_WRAPPER_TIMER_HPP
#define IOX_HOOFS_POSIX_WRAPPER_TIMER_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/design_pattern/creation.hpp"
#include "iceoryx_hoofs/internal/units/duration.hpp"
#include "iceoryx_hoofs/platform/signal.hpp"
#include "iceoryx_hoofs/platform/time.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <ctime>
#include <functional>
#include <limits>


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
    TIMEOUT_IS_ZERO,
    INTERNAL_LOGIC_ERROR
};

using namespace iox::units::duration_literals;


/// @brief Interface for timers on POSIX operating systems
/// @note Can't be copied or moved as operating system has a pointer to this object. It needs to be ensured that this
/// object lives longer than timeToWait, otherwise the operating system will unregister the timer
/// @concurrent not thread safe
///
/// @code
///     posix::Timer TiborTheTimer{100_ms, [&]() { fooBar++; }};
///
///     // Start a periodic timer
///     TiborTheTimer.start(true);
///     // [.. wait ..]
///     // Timer fires after 100_ms and calls the lambda which increments fooBar
///
///     TiborTheTimer.stop();
///
/// @endcode

/// This class will be DEPRECATED in the near future. In its current form there may still be potential races when
/// start/stop/restart are called concurrently (this includes the callback, which is executed in a separate thread).
/// The implementation also has too much overhead in the callback execution (due to execution logic and potentially
/// multiple callback threads).
///
/// It will be replaced with simpler versions for individual use cases, such as a CountdownTimer which can be used for
/// watchdog/keepalive purposes.
class Timer
{
  public:
    enum class RunMode
    {
        ONCE,
        PERIODIC
    };

    /// @brief
    ///     defines the behavior of the timer when the callback runtime is greater
    ///     than the periodic trigger time.
    ///     SKIP_TO_NEXT_BEAT skip callback and call it in the next cycle
    ///     IMMEDIATE call the callback right after the currently running callback is finished
    ///     TERMINATE terminates the process by calling the errorHandler with
    ///                 POSIX_TIMER__CALLBACK_RUNTIME_EXCEEDS_RETRIGGER_TIME
    enum class CatchUpPolicy
    {
        SKIP_TO_NEXT_BEAT,
        IMMEDIATE,
        TERMINATE
    };

  private:
    static constexpr size_t SIZE_OF_COMBINDED_INDEX_AND_DESCRIPTOR = sizeof(uint32_t);
    static constexpr size_t SIZE_OF_SIGVAL_INT = sizeof(int);
    static_assert(SIZE_OF_SIGVAL_INT >= SIZE_OF_COMBINDED_INDEX_AND_DESCRIPTOR, "size of sigval_int is to low");

    static constexpr uint32_t MAX_NUMBER_OF_CALLBACK_HANDLES = 100u;
    static_assert(MAX_NUMBER_OF_CALLBACK_HANDLES <= std::numeric_limits<uint8_t>::max(),
                  "number of callback handles exceeds max index value");
    class OsTimer;
    struct OsTimerCallbackHandle
    {
        static constexpr uint32_t MAX_DESCRIPTOR_VALUE{(1u << 24u) - 1u};
        static sigval indexAndDescriptorToSigval(uint8_t index, uint32_t descriptor) noexcept;
        static uint8_t sigvalToIndex(sigval intVal) noexcept;
        static uint32_t sigvalToDescriptor(sigval intVal) noexcept;

        void incrementDescriptor() noexcept;

        std::mutex m_accessMutex;

        /// @brief the descriptor is unique for a timer_t in OsTimer, if this handle is recycled, the descriptor needs
        /// to be incremented first
        std::atomic<uint32_t> m_descriptor{0u};
        // must be operator= otherwise it is undefined, see https://en.cppreference.com/w/cpp/atomic/ATOMIC_FLAG_INIT
        std::atomic_flag m_callbackIsAboutToBeExecuted = ATOMIC_FLAG_INIT;

        std::atomic<bool> m_inUse{false};
        std::atomic<bool> m_isTimerActive{false};
        std::atomic<uint64_t> m_timerInvocationCounter{0u};
        CatchUpPolicy m_catchUpPolicy{CatchUpPolicy::TERMINATE};
        OsTimer* m_timer{nullptr};
    };

    /// This class will be DEPRECATED in the near future.
    class OsTimer
    {
#ifdef __QNX__
        static constexpr timer_t INVALID_TIMER_ID = 0;
#else
        static constexpr timer_t INVALID_TIMER_ID = nullptr;
#endif
      public:
        /// @brief Wrapper that can be registered with the operating system
        static void callbackHelper(sigval data) noexcept;

        OsTimer(const units::Duration timeToWait, const std::function<void()>& callback) noexcept;

        OsTimer(const OsTimer&) = delete;
        OsTimer(OsTimer&&) = delete;
        OsTimer& operator=(const OsTimer&) = delete;
        OsTimer& operator=(OsTimer&&) = delete;

        /// @brief D'tor
        virtual ~OsTimer() noexcept;

        /// @brief Starts the timer
        ///
        /// The callback is called by the operating system after the time has expired.
        ///
        /// @param[in] runMode can be a periodic timer if set to RunMode::PERIODIC or
        ///                     it runs just once when it is set to RunMode::ONCE
        /// @param[in] CatchUpPolicy define behavior when callbackRuntime > timeToWait
        /// @note Shall only be called when callback is given
        cxx::expected<TimerError> start(const RunMode runMode, const CatchUpPolicy catchUpPolicy) noexcept;

        /// @brief Disarms the timer
        /// @note Shall only be called when callback is given, guarantee after stop() call is callback is immediately
        /// called or never at all
        cxx::expected<TimerError> stop() noexcept;

        /// @brief Disarms the timer, assigns a new timeToWait value and arms the timer
        /// @note Shall only be called when callback is given
        /// @param[in] runMode periodic can be a periodic timer if set to RunMode::PERIODIC or
        ///                     once when in RunMode::ONCE
        /// @param[in] CatchUpPolicy define behavior when callbackRuntime > timeToWait
        cxx::expected<TimerError>
        restart(const units::Duration timeToWait, const RunMode runMode, const CatchUpPolicy catchUpPolicy) noexcept;

        // @brief Returns the time until the timer expires the next time
        /// @note Shall only be called when callback is given
        cxx::expected<units::Duration, TimerError> timeUntilExpiration() noexcept;

        /// @brief In case the callback is not immediately called by the operating system, getOverruns() returns the
        /// additional overruns that happended in the delay interval
        /// @note Shall only be called when callback is given
        cxx::expected<uint64_t, TimerError> getOverruns() noexcept;

        /// @brief Returns true if the construction of the object was successful
        bool hasError() const noexcept;

        /// @brief Returns the error that occured on constructing the object
        TimerError getError() const noexcept;

      private:
        /// @brief Call the user-defined callback
        /// @note This call is wrapped in a plain C function
        void executeCallback() noexcept;

      private:
        /// @brief Duration after the timer calls the user-defined callback function
        units::Duration m_timeToWait;

        /// @brief Stores the user-defined callback
        std::function<void()> m_callback;

        /// @brief Identifier for the timer in the operating system
        timer_t m_timerId{INVALID_TIMER_ID};

        uint8_t m_callbackHandleIndex{0u};

        /// @todo will be obsolete with creation pattern
        /// @brief Bool that signals whether the object is fully initalized
        bool m_isInitialized{false};

        /// @todo creation pattern
        /// @brief If an error happened during creation the value is stored in here
        TimerError m_errorValue{TimerError::NO_ERROR};

        static OsTimerCallbackHandle s_callbackHandlePool[MAX_NUMBER_OF_CALLBACK_HANDLES];
    };

  public:
    /// @brief Creates a timer without an operating system callback
    ///
    /// Creates a light-weight timer object that can be used with
    ///               * hasExpiredComparedToCreationTime()
    ///               * resetCreationTime()
    ///
    /// @param[in] timeToWait - How long should be waited?
    /// @note Does not set up an operating system timer, but uses CLOCK_REALTIME instead
    /// @todo refactor this cTor and its functionality to a class called StopWatch
    Timer(const units::Duration timeToWait) noexcept;

    /// @brief Creates a timer with an operating system callback
    ///
    /// Initially the timer is stopped.
    ///
    /// @param[in] timeToWait - How long should be waited?
    /// @param[in] callback - Function called after timeToWait (User needs to ensure lifetime of function till stop()
    ///                       call)
    /// @note Operating systems needs a valid reference to this object, hence DesignPattern::Creation can't be used
    Timer(const units::Duration timeToWait, const std::function<void()>& callback) noexcept;

    /// @brief creates Duration from the result of clock_gettime(CLOCK_REALTIME, ...)
    /// @return if the clock_gettime call failed TimerError is returned otherwise Duration
    /// @todo maybe move this to a clock implementation?
    static cxx::expected<units::Duration, TimerError> now() noexcept;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer(const Timer& other) = delete;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer(Timer&& other) = delete;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer& operator=(const Timer& other) = delete;

    /// @brief Move or semantics are forbidden as address of object is not allowed to change
    Timer& operator=(Timer&& other) = delete;

    /// @brief D'tor
    virtual ~Timer() noexcept = default;

    /// @brief Starts the timer
    ///
    /// The callback is called by the operating system after the time has expired.
    ///
    /// @param[in] runMode for continuous callbacks PERIODIC otherwise ONCE
    /// @param[in] CatchUpPolicy define behavior when callbackRuntime > timeToWait
    /// @note Shall only be called when callback is given
    cxx::expected<TimerError> start(const RunMode runMode, const CatchUpPolicy catchUpPolicy) noexcept;

    /// @brief Disarms the timer
    /// @note Shall only be called when callback is given, guarantee after stop() call is callback is immediately
    /// called or never at all
    cxx::expected<TimerError> stop() noexcept;

    /// @brief Disarms the timer, assigns a new timeToWait value and arms the timer
    /// @param[in] timeToWait duration till the callback should be called
    /// @param[in] runMode for continuous callbacks PERIODIC otherwise ONCE
    /// @param[in] CatchUpPolicy define behavior when callbackRuntime > timeToWait
    /// @note Shall only be called when callback is given
    cxx::expected<TimerError>
    restart(const units::Duration timeToWait, const RunMode runMode, const CatchUpPolicy catchUpPolicy) noexcept;

    // @brief Returns the time until the timer expires the next time
    /// @note Shall only be called when callback is given
    cxx::expected<units::Duration, TimerError> timeUntilExpiration() noexcept;

    /// @brief In case the callback is not immediately called by the operating system, getOverruns() returns the
    /// additional overruns that happended in the delay interval
    /// @note Shall only be called when callback is given
    cxx::expected<uint64_t, TimerError> getOverruns() noexcept;

    /// @brief Returns true if the construction of the object was successful
    bool hasError() const noexcept;

    /// @brief Returns the error that occured on constructing the object
    TimerError getError() const noexcept;

  private:
    cxx::optional<OsTimer> m_osTimer;

    /// @brief Converts errnum to TimerError
    static cxx::error<TimerError> createErrorFromErrno(const int32_t errnum) noexcept;

    /// @brief Duration after the timer calls the user-defined callback function
    units::Duration m_timeToWait;

    /// @brief Time when the timer object was created
    units::Duration m_creationTime;

    /// @brief If an error happened during creation the value is stored in here
    TimerError m_errorValue{TimerError::NO_ERROR};
};

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_TIMER_HPP
