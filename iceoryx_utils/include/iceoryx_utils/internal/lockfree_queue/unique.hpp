#pragma once

#include <utility>

namespace iox
{
// remark: Is currently only used to implement unqiue index,
// which might be removed during the capacity change feature
// it is effectively a reduced optional with forbidden copies
// However, we do not need all the extra functionality of the optional
// and would need to account for the differences.

// The idea is to have a unique (value) resource which is only movable
// but otherwise behaves almost as the template type.

template <typename T>
class unique
{
  public:
    struct invalid_t
    {
        explicit constexpr invalid_t()
        {
        }
    };

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
        : m_valid(other.m_valid)
    {
        if (other.m_valid)
        {
            // note that we could move construct from other.m_value even if other.m_valid is false,
            // but this way it might be more efficient (depends on optimization and the underlying type)
            m_value = std::move(other.m_value);
            other.m_valid = false;
        }
    };

    unique& operator=(unique&& other)
    {
        if (this != &other)
        {
            m_valid = other.m_valid;
            if (other.m_valid)
            {
                m_value = std::move(other.m_value);
                other.m_valid = false;
            }
        }
        return *this;
    }

    const T& operator*() const
    {
        return m_value;
    }

    T release()
    {
        m_valid = false;
        return std::move(m_value);
    }

    operator const T&() const
    {
        return m_value;
    }

    bool isValid() const
    {
        return m_valid;
    }

  private:
    T m_value;
    bool m_valid{true};
};

} // namespace iox