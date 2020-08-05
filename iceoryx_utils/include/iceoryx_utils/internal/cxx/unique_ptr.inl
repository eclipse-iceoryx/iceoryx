
#ifndef IOX_UTILS_CXX_UNIQUE_PTR_INL
#define IOX_UTILS_CXX_UNIQUE_PTR_INL

#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace cxx
{

template<typename T>
unique_ptr<T>::unique_ptr(const  function_ref<void(T* const)> deleter) noexcept : m_deleter(deleter)
{}

template<typename T>
unique_ptr<T>::unique_ptr(ptr_t ptr, const function_ref<void(T* const)> deleter) noexcept : m_ptr(ptr), m_deleter(deleter)
{}

template<typename T>
unique_ptr<T>::unique_ptr(void* allocation, const function_ref<void(ptr_t const)> deleter) noexcept
    : m_ptr(reinterpret_cast<T*>(allocation)), m_deleter(deleter)
{}

template<typename T>
unique_ptr<T>::~unique_ptr() noexcept
{
    m_deleter(m_ptr);
}

/// Dereference the stored pointer.
template<typename T>
T unique_ptr<T>::operator*() noexcept
{
    return *get();
}

/// Return the stored pointer.
template<typename T>
T* unique_ptr<T>::operator->() noexcept
{
    return get();
}


template<typename T>
T* unique_ptr<T>::get() noexcept
{
  return m_ptr;
}

template<typename T>
T* unique_ptr<T>::release() noexcept
{
    auto ptr = m_ptr;
    m_ptr = nullptr;
    return ptr;
}

template<typename T>
void unique_ptr<T>::reset(T* ptr) noexcept
{
  if(m_ptr)
  {
      m_deleter(m_ptr);
  }
  m_ptr = ptr;
}

template<typename T>
void unique_ptr<T>::swap(unique_ptr<T>& other) noexcept
{
    // Release pointers from both instances.
    auto thisPtr = release();
    auto otherPtr = other.release();
    // Set new pointers on both instances.
    reset(otherPtr);
    other.reset(release());
}

} // namespace iox
} // namespace popo

#endif // IOX_UTILS_CXX_UNIQUE_PTR_INL
