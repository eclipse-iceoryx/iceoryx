#pragma once

#include <utility>

namespace iox
{
template <typename T>
class unique
{
  private:
    struct invalid_t
    {
        explicit constexpr invalid_t()
        {
        }
    };

  public:
    static constexpr invalid_t invalid{};

    template <typename... Args>
    unique(Args&&... args)
        : m_value(std::forward<Args>(args)...)
    {
    }

    unique()
        : m_value()
    {
    }

    unique(invalid_t)
        : m_valid(false)
    {
    }

    // no copies
    unique(const unique&) = delete;
    unique& operator=(const unique&) = delete;

    // only move
    unique(unique&& other)
        : m_value(std::move(other.m_value))
    {
        other.m_valid = false;
    };

    unique& operator=(unique&& other)
    {
        if (this != &other)
        {
            m_value = std::move(other.m_value);
            m_valid = other.m_valid;
            other.m_valid = false;
        }
        return *this;
    }

    T& operator*()
    {
        return m_value;
    }

    const T& operator*() const
    {
        return m_value;
    }

    T get()
    {
        m_valid = false;
        return std::move(m_value);
    }

    operator const T&() const
    {
        return m_value;
    }

    bool is_valid() const
    {
        return m_valid;
    }

    // interferes with operator const T& for some types, we cannot have both (which do we want?)
    // operator bool()
    // {
    //     return m_valid;
    // }

  private:
    // note: storing the value enforces a (default) ctor
    T m_value;
    bool m_valid{true};
};
} // namespace iox