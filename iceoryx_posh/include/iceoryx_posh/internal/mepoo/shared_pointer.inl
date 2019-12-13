// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

namespace iox
{
namespace mepoo
{
template <typename T>
template <typename... Targs>
inline SharedPointer<T>::SharedPointer(const SharedChunk& chunk, Targs&&... args)
    : m_chunk(chunk)
{
    if (chunk.m_chunkManagement != nullptr)
    {
        new (chunk.m_chunkManagement->m_chunkHeader->payload()) T(std::forward<Targs>(args)...);
        this->m_isInitialized = true;
    }
    else
    {
        this->m_isInitialized = false;
        this->m_errorValue = SharedPointerError::SharedChunkIsEmpty;
    }
}

template <typename T>
inline SharedPointer<T>::~SharedPointer()
{
    deleteManagedObjectIfNecessary();
}

template <typename T>
inline SharedPointer<T>& SharedPointer<T>::operator=(const SharedPointer& rhs)
{
    if (this != &rhs)
    {
        deleteManagedObjectIfNecessary();
        m_chunk = rhs.m_chunk;
    }
    return *this;
}

template <typename T>
inline SharedPointer<T>& SharedPointer<T>::operator=(SharedPointer&& rhs)
{
    if (this != &rhs)
    {
        deleteManagedObjectIfNecessary();
        m_chunk = std::move(rhs.m_chunk);
    }
    return *this;
}

template <typename T>
inline void SharedPointer<T>::deleteManagedObjectIfNecessary()
{
    if (m_chunk.m_chunkManagement != nullptr
        && m_chunk.m_chunkManagement->m_referenceCounter.load(std::memory_order_relaxed) == 2)
    {
        static_cast<T*>(m_chunk.m_chunkManagement->m_chunkHeader->payload())->~T();
    }
}

template <typename T>
inline T* SharedPointer<T>::get()
{
    return static_cast<T*>(m_chunk.m_chunkManagement->m_chunkHeader->payload());
}

template <typename T>
inline const T* SharedPointer<T>::get() const
{
    return const_cast<SharedPointer*>(this)->get();
}

template <typename T>
inline T* SharedPointer<T>::operator->()
{
    return static_cast<T*>(m_chunk.m_chunkManagement->m_chunkHeader->payload());
}

template <typename T>
inline const T* SharedPointer<T>::operator->() const
{
    return const_cast<SharedPointer*>(this)->operator->();
}

template <typename T>
inline T& SharedPointer<T>::operator*()
{
    return *static_cast<T*>(m_chunk.m_chunkManagement->m_chunkHeader->payload());
}

template <typename T>
inline const T& SharedPointer<T>::operator*() const
{
    return const_cast<SharedPointer*>(this)->operator*();
}


} // namespace mepoo
} // namespace iox
