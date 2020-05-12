#pragma once

#include <stdint.h>

namespace iox
{
// @todo: add more functionality (policies for cache line size, redzoning), rename in a suitable way
template <typename T, uint64_t Capacity, typename index_t = uint64_t>
class Buffer
{
  public:
    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    T& operator[](index_t index) noexcept
    {
        return *toPtr(index);
    }

    const T& operator[](index_t index) const noexcept
    {
        return *toPtr(index);
    }

    T* ptr(index_t index) noexcept
    {
        return toPtr(index);
    }

    const T* ptr(index_t index) const noexcept
    {
        return toPtr(index);
    }

    uint64_t capacity()
    {
        return Capacity;
    }

  private:
    using byte_t = uint8_t;

    alignas(alignof(T)) byte_t m_buffer[Capacity * sizeof(T)];

    T* toPtr(index_t index) const noexcept
    {
        auto ptr = &(m_buffer[index * sizeof(T)]);
        return reinterpret_cast<T*>(const_cast<byte_t*>(ptr));
    }
};

} // namespace iox
