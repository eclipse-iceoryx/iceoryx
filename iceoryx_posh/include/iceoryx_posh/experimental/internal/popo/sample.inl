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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_INL
#define IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_INL

namespace iox
{
namespace popo
{
template <typename T>
Sample<T>::Sample(cxx::unique_ptr<T>&& samplePtr) noexcept
    : m_samplePtr(std::move(samplePtr)){};

template <typename T>
Sample<T>::Sample(std::nullptr_t) noexcept {};

template <typename T>
Sample<T>& Sample<T>::operator=(Sample<T>&& rhs)
{
    if (this != &rhs)
    {
        m_samplePtr = std::move(rhs.m_samplePtr);
    }
    return *this;
}

template <typename T>
Sample<T>::Sample(Sample<T>&& rhs)
{
    *this = std::move(rhs);
}

template <typename T>
Sample<T>::~Sample()
{
    m_samplePtr = nullptr;
}

template <typename T>
Sample<T>& Sample<T>::operator=(std::nullptr_t) noexcept
{
    m_samplePtr = nullptr; // The pointer will take care of cleaning up resources.
    return *this;
}

template <typename T>
T* Sample<T>::operator->() const noexcept
{
    return get();
}

template <typename T>
T* Sample<T>::get() const noexcept
{
    return m_samplePtr.get();
}

template <typename T>
const mepoo::ChunkHeader* Sample<T>::header() const noexcept
{
    return mepoo::convertPayloadPointerToChunkHeader(m_samplePtr.get());
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_INL
