// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_POPO_SAMPLE_INL
#define IOX_POSH_POPO_SAMPLE_INL

namespace iox
{
namespace popo
{
namespace internal
{
template <typename T>
inline SamplePrivateData<T>::SamplePrivateData(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher) noexcept
    : samplePtr(std::move(samplePtr))
    , publisherRef(publisher)
{
}

template <typename T>
inline SamplePrivateData<const T>::SamplePrivateData(cxx::unique_ptr<const T>&& samplePtr) noexcept
    : samplePtr(std::move(samplePtr))
{
}
} // namespace internal

template <typename T>
template <typename S, typename>
inline Sample<T>::Sample(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher) noexcept
    : m_members({std::move(samplePtr), publisher})
{
}

template <typename T>
template <typename S, typename>
inline Sample<T>::Sample(cxx::unique_ptr<T>&& samplePtr) noexcept
    : m_members(std::move(samplePtr))
{
}

template <typename T>
inline Sample<T>::Sample(std::nullptr_t) noexcept
{
    m_members.samplePtr.reset();
}

template <typename T>
template <typename S, typename>
inline T* Sample<T>::operator->() noexcept
{
    return get();
}

template <typename T>
inline const T* Sample<T>::operator->() const noexcept
{
    return get();
}

template <typename T>
template <typename S, typename>
inline S& Sample<T>::operator*() noexcept
{
    return *get();
}

template <typename T>
inline const T& Sample<T>::operator*() const noexcept
{
    return *get();
}

template <typename T>
inline Sample<T>::operator bool() const noexcept
{
    return get() != nullptr;
}

template <typename T>
template <typename S, typename>
inline T* Sample<T>::get() noexcept
{
    return m_members.samplePtr.get();
}

template <typename T>
inline const T* Sample<T>::get() const noexcept
{
    return m_members.samplePtr.get();
}

template <typename T>
template <typename S, typename>
inline mepoo::ChunkHeader* Sample<T>::getHeader() noexcept
{
    return mepoo::ChunkHeader::fromPayload(m_members.samplePtr.get());
}

template <typename T>
inline const mepoo::ChunkHeader* Sample<T>::getHeader() const noexcept
{
    return mepoo::ChunkHeader::fromPayload(m_members.samplePtr.get());
}

template <typename T>
template <typename S, typename>
inline void Sample<T>::publish() noexcept
{
    if (m_members.samplePtr)
    {
        m_members.publisherRef.get().publish(std::move(*this));
    }

    else
    {
        /// @todo Notify caller of attempt to publish invalid chunk. Or something ?
    }
}

template <typename T>
template <typename S, typename>
inline void Sample<T>::release() noexcept
{
    m_members.samplePtr.reset();
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_SAMPLE_INL
