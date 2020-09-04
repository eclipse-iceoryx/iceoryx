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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"

namespace iox {
namespace popo {

using uid_t = uint64_t;

struct Untyped {};

template<typename T, typename port_t = iox::popo::SenderPort>
class BasePublisher;

// ======================================== Sample ======================================== //

// Sample needs to be specialized when T = void, since we are working with an untyped publisher in this case ..?
template <typename T>
class Sample
{
public:
    Sample(cxx::unique_ptr<T>&& samplePtr, BasePublisher<T>& publisher)
        : m_samplePtr(std::move(samplePtr)), m_publisherRef(publisher)
    {};

    Sample(std::nullptr_t) noexcept {};

    Sample(const Sample&) = delete;
    Sample& operator=(const Sample&) = delete;

    Sample& operator=(Sample<T>&& rhs)
    {
        if(this != &rhs)
        {
            m_samplePtr.reset(rhs.m_samplePtr.release());
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
            m_samplePtr.release();      // Release ownership of the sample since it has been published.
            m_hasOwnership = false;
        }

        else
        {
            /// @todo Notify caller of attempt to publish invalid chunk.
        }
    }

private:
    bool m_hasOwnership = true;
    cxx::unique_ptr<T> m_samplePtr = nullptr;
    std::reference_wrapper<BasePublisher<T>> m_publisherRef;
};

// ======================================== Base Publisher ======================================== //

enum class SampleRecallError : uint8_t
{
    NO_PREVIOUS_CHUNK,
    CHUNK_ALREADY_CLAIMED
};

template<typename T, typename port_t>
class BasePublisher
{
public:

    BasePublisher(const BasePublisher& other) = delete;
    BasePublisher& operator=(const BasePublisher&) = delete;
    BasePublisher(BasePublisher&& rhs) = default;
    BasePublisher& operator=(BasePublisher&& rhs) = default;
    ~BasePublisher() = default;

    uid_t uid() const noexcept;
    cxx::expected<Sample<T>, AllocationError> loan(uint64_t size) noexcept;
    void release(Sample<T>& sample) noexcept;
    cxx::expected<AllocationError> publish(Sample<T>& sample) noexcept;
    cxx::expected<SampleRecallError> previousSample() const noexcept;

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;

protected:
    BasePublisher(const capro::ServiceDescription& service);

protected:
    port_t m_port{nullptr};
    bool m_useDynamicPayloadSize = true;

private:

};

// ======================================== Typed Publisher ======================================== //

template<typename T>
class TypedPublisher : protected iox::popo::BasePublisher<T>
{
public:

    TypedPublisher(const capro::ServiceDescription& service);
    TypedPublisher(const TypedPublisher& other) = delete;
    TypedPublisher& operator=(const TypedPublisher&) = delete;
    TypedPublisher(TypedPublisher&& rhs) = default;
    TypedPublisher& operator=(TypedPublisher&& rhs) = default;
    ~TypedPublisher() = default;

    uid_t uid() const noexcept;

    cxx::expected<Sample<T>, AllocationError> loan() noexcept;
    void release(Sample<T>& sample) noexcept;
    cxx::expected<AllocationError> publish(Sample<T>& sample) noexcept;
    cxx::expected<AllocationError> publishCopyOf(const T& val) noexcept; /// @todo - move to typed API
    cxx::expected<SampleRecallError> previousSample() const noexcept;

    // This does not quite work yet. Need to understand how to identify generic callables in C++11.
    template<typename Callable, typename... ArgTypes>
    cxx::expected<AllocationError> publishResultOf(Callable c, ArgTypes... args) noexcept;

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;

};

// ======================================== Untyped Publisher ======================================== //

class UntypedPublisher : protected iox::popo::BasePublisher<void>
{
public:

    UntypedPublisher(const capro::ServiceDescription& service);
    UntypedPublisher(const UntypedPublisher& other) = delete;
    UntypedPublisher& operator=(const UntypedPublisher&) = delete;
    UntypedPublisher(UntypedPublisher&& rhs) = default;
    UntypedPublisher& operator=(UntypedPublisher&& rhs) = default;
    ~UntypedPublisher() = default;

    uid_t uid() const noexcept;

    cxx::expected<Sample<void>, AllocationError> loan(uint64_t size) noexcept;
    void release(Sample<void>& sample) noexcept;
    cxx::expected<AllocationError> publish(Sample<void>& sample) noexcept;
    cxx::expected<SampleRecallError> previousSample() const noexcept;

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;

};

}
}

#include "iceoryx_posh/experimental/internal/popo/publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP
