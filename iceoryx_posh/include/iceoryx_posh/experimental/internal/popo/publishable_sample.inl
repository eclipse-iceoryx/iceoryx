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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_PUBLISHABLE_SAMPLE_INL
#define IOX_EXPERIMENTAL_POSH_POPO_PUBLISHABLE_SAMPLE_INL

namespace iox
{
namespace popo
{
template <typename T>
PublishableSample<T>::PublishableSample(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher)
    : Sample<T>(std::move(samplePtr))
    , m_publisherRef(publisher){};

template <typename T>
PublishableSample<T>::PublishableSample(std::nullptr_t) noexcept {};

template <typename T>
PublishableSample<T>& PublishableSample<T>::operator=(PublishableSample<T>&& rhs)
{
    if (this != &rhs)
    {
        m_publisherRef = rhs.m_publisherRef;
        m_hasOwnership = rhs.m_hasOwnership;
    }
    return *this;
}

template <typename T>
PublishableSample<T>::PublishableSample(PublishableSample<T>&& rhs)
    : Sample<T>(std::move(rhs))
    , m_publisherRef(rhs.m_publisherRef) // Need to initialize references in initializer list.
{
    *this = std::move(rhs);
}

template <typename T>
PublishableSample<T>& PublishableSample<T>::operator=(std::nullptr_t) noexcept
{
    m_hasOwnership = false;
    Sample<T>::m_samplePtr = nullptr; // The pointer will take care of cleaning up resources.
    return *this;
}

template <typename T>
T* PublishableSample<T>::get() noexcept
{
    if (m_hasOwnership)
    {
        return Sample<T>::get();
    }
    else
    {
        return nullptr;
    }
}

template <typename T>
void PublishableSample<T>::publish() noexcept
{
    if (m_hasOwnership)
    {
        m_publisherRef.get().publish(*this);
        m_hasOwnership = false;
        Sample<T>::m_samplePtr.release(); // Release ownership of the sample since it has been published.
    }

    else
    {
        /// @todo Notify caller of attempt to publish invalid chunk.
    }
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHABLE_SAMPLE_INL
