#pragma once

#include <stdint.h>

namespace iox
{
// @todo: add more functionality (policies for cache line size, redzoning), rename in a suitable way
template <typename ElementType, uint64_t Capacity, typename index_t = uint64_t>
class Buffer
{
  public:
    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    ElementType& operator[](index_t index) noexcept
    {
        return *toPtr(index);
    }

    const ElementType& operator[](index_t index) const noexcept
    {
        return *toPtr(index);
    }

    ElementType* ptr(index_t index) noexcept
    {
        return toPtr(index);
    }

    const ElementType* ptr(index_t index) const noexcept
    {
        return toPtr(index);
    }

    uint64_t capacity()
    {
        return Capacity;
    }

  private:
    using byte_t = uint8_t;

    alignas(alignof(ElementType)) byte_t m_buffer[Capacity * sizeof(ElementType)];

    ElementType* toPtr(index_t index) const noexcept
    {
        auto ptr = &(m_buffer[index * sizeof(ElementType)]);
        return reinterpret_cast<ElementType*>(const_cast<byte_t*>(ptr));
    }
};

} // namespace iox
