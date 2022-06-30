#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/runtime_error.hpp"

#include <mutex>
#include <vector>

// simple abstraction to track the errors in tests without requiring exceptions

namespace eh
{
class ErrorStorage
{
  public:
    void reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_errors.clear();
    }

    void add(const RuntimeError& error)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_errors.push_back(error);
    }

    uint32_t count(const RuntimeError& error) const
    {
        uint32_t n{0};

        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& e : m_errors)
        {
            if (e == error)
            {
                ++n;
            }
        }
        return n;
    }

  private:
    std::vector<RuntimeError> m_errors;
    mutable std::mutex m_mutex;
};

} // namespace eh