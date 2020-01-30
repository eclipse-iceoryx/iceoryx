#include "iceoryx_utils/internal/relocatable_pointer/atomic_relocatable_ptr.hpp"

namespace iox
{
template <typename T>
inline atomic_relocatable_ptr<T>::atomic_relocatable_ptr(const T* ptr)
    : m_offset(computeOffset(ptr))
{
}

template <typename T>
inline atomic_relocatable_ptr<T>& atomic_relocatable_ptr<T>::operator=(const T* ptr) noexcept
{
    m_offset.store(computeOffset(ptr), std::memory_order_relaxed);
    return *this;
}

template <typename T>
inline T* atomic_relocatable_ptr<T>::operator->() const noexcept
{
    return computeRawPtr();
}

template <typename T>
inline T& atomic_relocatable_ptr<T>::operator*() const noexcept
{
    return *computeRawPtr();
}


template <typename T>
inline atomic_relocatable_ptr<T>::operator T*() const noexcept
{
    return computeRawPtr();
}

template <typename T>
inline T* atomic_relocatable_ptr<T>::computeRawPtr() const
{
    auto offset = m_offset.load(std::memory_order_relaxed);
    if (offset == NULL_POINTER_OFFSET)
    {
        return nullptr;
    }

    return reinterpret_cast<T*>(reinterpret_cast<offset_t>(&m_offset) - offset);
}

template <typename T>
inline typename atomic_relocatable_ptr<T>::offset_t atomic_relocatable_ptr<T>::computeOffset(const void* ptr) const
{
    if (ptr == nullptr)
    {
        return NULL_POINTER_OFFSET;
    }
    return reinterpret_cast<offset_t>(&m_offset) - reinterpret_cast<offset_t>(ptr);
}
}