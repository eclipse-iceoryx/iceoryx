// minimalistic relocatable pointer, to find the issues with the existing one
// and create a minimal API to be used for relocatable linked structures

#include <cstdint>
#include <limits>

namespace iox
{
template <typename T>
class relocatable_ptr
{
    using offset_t = uint64_t;

    // we can do this since the value at the reference address has size > 1 Byte (meaning the address is unusable)
    static constexpr offset_t NULL_POINTER_OFFSET = 1;

  public:
    relocatable_ptr(T* ptr)
    {
        m_ptr = ptr;
        m_offset = toOffset(ptr);
        // print();
    }

    relocatable_ptr(const relocatable_ptr& other)
    {
        m_ptr = other.rawPtr();
        m_offset = toOffset(m_ptr);
        // print();
    }

    relocatable_ptr& operator=(T* ptr)
    {
        m_ptr = ptr;
        m_offset = toOffset(ptr);

        // print();

        if (ptr != rawPtr())
        {
            std::cout << "conversion error" << std::endl;
        }
        return *this;
    }

    relocatable_ptr& operator=(const relocatable_ptr& rhs)
    {
        if (&rhs != this)
        {
            *this = rhs.rawPtr();
        }
        return *this;
    }

    T& operator*()
    {
        return *rawPtr();
    }

    T* operator->()
    {
        return rawPtr();
    }

    operator T*()
    {
        return rawPtr();
    }

    void print()
    {
        std::cout << "self " << self() << " offset " << m_offset << " raw " << rawPtr() << std::endl;
    }


  private:
    T* m_ptr; // TODO: remove in a cleaned up version
    offset_t m_offset;
    T* m_ptr2;

    T* rawPtr() const
    {
        return fromOffset(m_offset);
    }

    offset_t self() const
    {
        return reinterpret_cast<offset_t>(&m_offset);
    }

    T* fromOffset(offset_t offset) const
    {
        if (offset == NULL_POINTER_OFFSET)
        {
            return nullptr;
        }
        // return m_ptr;

        return reinterpret_cast<T*>(self() - offset);
    }

    offset_t toOffset(void* ptr) const
    {
        if (ptr == nullptr)
        {
            return NULL_POINTER_OFFSET;
        }
        return self() - reinterpret_cast<offset_t>(ptr);
    }
};
} // namespace iox