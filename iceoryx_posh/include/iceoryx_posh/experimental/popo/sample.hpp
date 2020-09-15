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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_HPP

#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox {
namespace popo {

template<typename T>
class PublisherInterface;

template <typename T>
class Sample
{
public:
    Sample(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher)
        : m_samplePtr(std::move(samplePtr)), m_publisherRef(publisher)
    {};

    Sample(std::nullptr_t) noexcept {};

    Sample(const Sample&) = delete;
    Sample& operator=(const Sample&) = delete;

    Sample& operator=(Sample<T>&& rhs)
    {
        if(this != &rhs)
        {
            m_samplePtr = std::move(rhs.m_samplePtr);
            m_publisherRef = rhs.m_publisherRef;
            m_hasOwnership = rhs.m_hasOwnership;
        }
        return *this;
    }
    Sample(Sample<T>&& rhs) : m_publisherRef(rhs.m_publisherRef) // Need to initialize references in initializer list.
    {
        *this = std::move(rhs);
    }

    ~Sample()
    {
        m_samplePtr = nullptr;
    }
    ///
    /// @brief operator = Clear the sample.
    /// @return
    ///
    Sample& operator=(std::nullptr_t) noexcept
    {
      m_samplePtr = nullptr;    // The pointer will take care of cleaning up resources.
      m_hasOwnership = false;
      return *this;
    }
    ///
    /// @brief operator -> Transparent access to the underlying pointer.
    /// @return
    ///
    T* operator->() noexcept
    {
        return get();
    }
    ///
    /// @brief allocation Access to the memory allocated to the sample.
    /// @return
    ///
    T* get() noexcept
    {
        if(m_hasOwnership)
        {
            return m_samplePtr.get();
        }
        else
        {
            return nullptr;
        }
    }
    ///
    /// @brief publish Publish the sample.
    ///
    void publish() noexcept
    {
        if(m_hasOwnership)
        {
            m_publisherRef.get().publish(*this);
            m_hasOwnership = false;
            m_samplePtr.release();      // Release ownership of the sample since it has been published.
        }

        else
        {
            /// @todo Notify caller of attempt to publish invalid chunk.
        }
    }

private:
    bool m_hasOwnership{true};
    cxx::unique_ptr<T> m_samplePtr{nullptr};
    std::reference_wrapper<PublisherInterface<T>> m_publisherRef;
};

#include "iceoryx_posh/experimental/internal/popo/sample.inl"

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_SAMPLE_HPP

