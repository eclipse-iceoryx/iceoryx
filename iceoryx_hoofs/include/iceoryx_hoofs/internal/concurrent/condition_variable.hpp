/*
 * ConditionVariable implementation to fix monotonic_clock.
 *
 * There is a problem with std::condition_variable in lower versions of gcc,
 * which actually uses std::chrono::system_clock instead of std::chrono::steady_clock.
 * Bug 41861 (DR887) - [DR887][C++0x] <condition_variable> does not use monotonic_clock
 *
 * Since new versions of gcc cannot be used, stdext::condition_variable can only be
 * manually implemented instead of C++11's std::condition_variable.
 *
 * This problem is mainly solved by using the POSIX function pthread_condattr_setclock to
 * set the attribute of the condition variable to CLOCK_MONOTONIC.
 */

#pragma once

#include <condition_variable>
#include <utility>

#ifdef __unix__

#include <pthread.h>

#include <chrono>
#include <mutex>

namespace stdext {

class ConditionVariable final {
 public:
  using native_handle_type = pthread_cond_t*;

  ConditionVariable(const ConditionVariable&) noexcept = delete;

  ConditionVariable& operator=(const ConditionVariable&) noexcept = delete;

  ConditionVariable() noexcept;

  ~ConditionVariable() noexcept;

  void notify_one() noexcept;

  void notify_all() noexcept;

  void wait(std::unique_lock<std::mutex>& lock) noexcept;

  template <typename PredicateT>
  void wait(std::unique_lock<std::mutex>& lock, PredicateT p) noexcept;

  template <typename DurationT>
  std::cv_status wait_until(std::unique_lock<std::mutex>& lock,
                            const std::chrono::time_point<std::chrono::steady_clock, DurationT>& atime) noexcept;

  template <typename DurationT>
  std::cv_status wait_until(std::unique_lock<std::mutex>& lock,
                            const std::chrono::time_point<std::chrono::system_clock, DurationT>& atime) noexcept;

  template <typename ClockT, typename DurationT>
  std::cv_status wait_until(std::unique_lock<std::mutex>& lock,
                            const std::chrono::time_point<ClockT, DurationT>& atime) noexcept;

  template <typename ClockT, typename DurationT, typename PredicateT>
  bool wait_until(std::unique_lock<std::mutex>& lock, const std::chrono::time_point<ClockT, DurationT>& atime,
                  PredicateT p) noexcept;

  template <typename RepT, typename PeriodT>
  std::cv_status wait_for(std::unique_lock<std::mutex>& lock,
                          const std::chrono::duration<RepT, PeriodT>& rtime) noexcept;

  template <typename RepT, typename PeriodT, typename PredicateT>
  bool wait_for(std::unique_lock<std::mutex>& lock, const std::chrono::duration<RepT, PeriodT>& rtime,
                PredicateT p) noexcept;

  native_handle_type native_handle() noexcept;

 private:
  template <typename ToDurT, typename RepT, typename PeriodT>
  static constexpr ToDurT ceil(const std::chrono::duration<RepT, PeriodT>& d) noexcept;

  template <typename TpT, typename UpT>
  static constexpr TpT ceil_impl(const TpT& t, const UpT& u) noexcept;

  template <typename DurationT>
  std::cv_status wait_until_impl(std::unique_lock<std::mutex>& lock,
                                 const std::chrono::time_point<std::chrono::steady_clock, DurationT>& atime) noexcept;

  pthread_cond_t cond_{};
};

////////////////////////////////////////////////////////////////
/// Details
////////////////////////////////////////////////////////////////

inline ConditionVariable::ConditionVariable() noexcept {
  pthread_condattr_t attr;

  pthread_condattr_init(&attr);
  pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  pthread_cond_init(&cond_, &attr);
  pthread_condattr_destroy(&attr);
}

inline ConditionVariable::~ConditionVariable() noexcept { pthread_cond_destroy(&cond_); }

inline void ConditionVariable::notify_one() noexcept { pthread_cond_signal(&cond_); }

inline void ConditionVariable::notify_all() noexcept { pthread_cond_broadcast(&cond_); }

inline void ConditionVariable::wait(std::unique_lock<std::mutex>& lock) noexcept {
  pthread_cond_wait(&cond_, lock.mutex()->native_handle());
}

template <typename PredicateT>
inline void ConditionVariable::wait(std::unique_lock<std::mutex>& lock, PredicateT p) noexcept {
  while (!p()) {
    wait(lock);
  }
}

template <typename DurationT>
inline std::cv_status ConditionVariable::wait_until(
    std::unique_lock<std::mutex>& lock,
    const std::chrono::time_point<std::chrono::steady_clock, DurationT>& atime) noexcept {
  return wait_until_impl(lock, atime);
}

template <typename DurationT>
inline std::cv_status ConditionVariable::wait_until(
    std::unique_lock<std::mutex>& lock,
    const std::chrono::time_point<std::chrono::system_clock, DurationT>& atime) noexcept {
  return wait_until<std::chrono::system_clock, DurationT>(lock, atime);
}

template <typename ClockT, typename DurationT>
inline std::cv_status ConditionVariable::wait_until(std::unique_lock<std::mutex>& lock,
                                                    const std::chrono::time_point<ClockT, DurationT>& atime) noexcept {
  const typename ClockT::time_point c_entry = ClockT::now();
  const std::chrono::steady_clock::time_point s_entry = std::chrono::steady_clock::now();
  const auto delta = atime - c_entry;
  const auto s_atime = s_entry + ceil<std::chrono::steady_clock::duration>(delta);

  if (wait_until_impl(lock, s_atime) == std::cv_status::no_timeout) {
    return std::cv_status::no_timeout;
  }

  if (ClockT::now() < atime) {
    return std::cv_status::no_timeout;
  }

  return std::cv_status::timeout;
}

template <typename ClockT, typename DurationT, typename PredicateT>
inline bool ConditionVariable::wait_until(std::unique_lock<std::mutex>& lock,
                                          const std::chrono::time_point<ClockT, DurationT>& atime,
                                          PredicateT p) noexcept {
  while (!p()) {
    if (wait_until(lock, atime) == std::cv_status::timeout) {
      return p();
    }
  }

  return true;
}

template <typename RepT, typename PeriodT>
inline std::cv_status ConditionVariable::wait_for(std::unique_lock<std::mutex>& lock,
                                                  const std::chrono::duration<RepT, PeriodT>& rtime) noexcept {
  return wait_until(lock, std::chrono::steady_clock::now() + ceil<std::chrono::steady_clock::duration>(rtime));
}

template <typename RepT, typename PeriodT, typename PredicateT>
inline bool ConditionVariable::wait_for(std::unique_lock<std::mutex>& lock,
                                        const std::chrono::duration<RepT, PeriodT>& rtime, PredicateT p) noexcept {
  return wait_until(lock, std::chrono::steady_clock::now() + ceil<std::chrono::steady_clock::duration>(rtime),
                    std::move(p));
}

inline ConditionVariable::native_handle_type ConditionVariable::native_handle() noexcept { return &cond_; }

template <typename ToDurT, typename RepT, typename PeriodT>
inline constexpr ToDurT ConditionVariable::ceil(const std::chrono::duration<RepT, PeriodT>& d) noexcept {  // NOLINT
  return ceil_impl(std::chrono::duration_cast<ToDurT>(d), d);
}

template <typename TpT, typename UpT>
inline constexpr TpT ConditionVariable::ceil_impl(const TpT& t, const UpT& u) noexcept {  // NOLINT
  return (t < u) ? (t + TpT{1}) : t;
}

template <typename DurationT>
inline std::cv_status ConditionVariable::wait_until_impl(
    std::unique_lock<std::mutex>& lock,
    const std::chrono::time_point<std::chrono::steady_clock, DurationT>& atime) noexcept {
  auto s = std::chrono::time_point_cast<std::chrono::seconds>(atime);
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(atime - s);

  struct timespec ts = {static_cast<std::time_t>(s.time_since_epoch().count()),  // NOLINT
                        static_cast<long>(ns.count())};                          // NOLINT

  pthread_cond_timedwait(&cond_, lock.mutex()->native_handle(), &ts);

  return (std::chrono::steady_clock::now() < atime ? std::cv_status::no_timeout : std::cv_status::timeout);
}

using condition_variable = ConditionVariable;  // NOLINT

}  // namespace stdext

#else

namespace stdext {

using ConditionVariable = std::condition_variable;  // NOLINT
using condition_variable = ConditionVariable;       // NOLINT

}  // namespace stdext

#endif
