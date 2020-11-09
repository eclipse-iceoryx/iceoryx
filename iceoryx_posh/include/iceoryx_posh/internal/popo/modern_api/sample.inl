// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_POSH_POPO_SAMPLE_INL
#define IOX_POSH_POPO_SAMPLE_INL

namespace iox
{
namespace popo
{
// ============================== Sample<T> ========================= //
template <typename T>
inline Sample<T>::Sample(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher)
    : m_samplePtr(std::move(samplePtr))
    , m_publisherRef(publisher){};

template <typename T>
inline Sample<T>& Sample<T>::operator=(Sample<T>&& rhs)
{
    if (this != &rhs)
    {
        m_publisherRef = rhs.m_publisherRef;
        m_samplePtr = std::move(rhs.m_samplePtr);
        rhs.m_samplePtr = nullptr;
    }
    return *this;
}

template <typename T>
inline Sample<T>::Sample(Sample<T>&& rhs)
    : m_publisherRef(rhs.m_publisherRef) // Need to initialize references in initializer list.
{
    *this = std::move(rhs);
}

template <typename T>
inline Sample<T>::~Sample()
{
    m_samplePtr = nullptr;
}

template <typename T>
inline Sample<T>::Sample(std::nullptr_t) noexcept
{
    m_samplePtr = nullptr; // The pointer will take care of cleaning up resources.
};

template <typename T>
inline T* Sample<T>::operator->() noexcept
{
    return get();
}

template <typename T>
inline T* Sample<T>::get() noexcept
{
    return m_samplePtr.get();
}

template <typename T>
inline mepoo::ChunkHeader* Sample<T>::getHeader() noexcept
{
    return mepoo::convertPayloadPointerToChunkHeader(m_samplePtr.get());
}

template <typename T>
inline void Sample<T>::publish() noexcept
{
    if (m_samplePtr)
    {
        m_publisherRef.get().publish(std::move(*this));
    }

    else
    {
        /// @todo Notify caller of attempt to publish invalid chunk. Or something ?
    }
}

template <typename T>
inline void Sample<T>::release() noexcept
{
    m_samplePtr.release();
}

// ============================== Sample<const T> ========================= //

template <typename T>
inline Sample<const T>::Sample(cxx::unique_ptr<T>&& samplePtr) noexcept
    : m_samplePtr(std::move(samplePtr)){};

template <typename T>
inline Sample<const T>::Sample(std::nullptr_t) noexcept
{
    m_samplePtr = nullptr; // The pointer will take care of cleaning up resources.
};

template <typename T>
inline Sample<const T>& Sample<const T>::operator=(Sample<const T>&& rhs)
{
    if (this != &rhs)
    {
        m_samplePtr = std::move(rhs.m_samplePtr);
        rhs.m_samplePtr = nullptr;
    }
    return *this;
}

template <typename T>
inline Sample<const T>::Sample(Sample<const T>&& rhs)
{
    *this = std::move(rhs);
}

template <typename T>
inline Sample<const T>::~Sample()
{
    m_samplePtr = nullptr;
}

template <typename T>
inline const T* Sample<const T>::operator->() noexcept
{
    return get();
}

template <typename T>
inline const T* Sample<const T>::get() noexcept
{
    return m_samplePtr.get();
}

template <typename T>
inline const mepoo::ChunkHeader* Sample<const T>::getHeader() noexcept
{
    return mepoo::convertPayloadPointerToChunkHeader(m_samplePtr.get());
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_SAMPLE_INL
