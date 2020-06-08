#pragma once

#include <stdint.h>

namespace iox
{
// remark: we can add more functionality (policies for cache line size, redzoning)

template <typename ElementType, uint64_t Capacity, typename index_t = uint64_t>
class Buffer
{
  public:
    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    ElementType& operator[](const index_t index) noexcept
    {
        return *toPtr(index);
    }

    const ElementType& operator[](const index_t index) const noexcept
    {
        return *toPtr(index);
    }

    ElementType* ptr(const index_t index) noexcept
    {
        return toPtr(index);
    }

    const ElementType* ptr(const index_t index) const noexcept
    {
        return toPtr(index);
    }

    uint64_t capacity() const noexcept
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
