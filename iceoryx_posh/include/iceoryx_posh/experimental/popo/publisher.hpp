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
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"

namespace iox {
namespace popo {

using uid_t = uint64_t;

template<typename Publisher>
struct get_port_type;

template<template<typename, typename> class Publisher, typename T, typename PortType>
struct get_port_type<Publisher<T, PortType>>
{
    using type = PortType;
};

template<typename T>
class PublisherInterface;

// ======================================== Sample ======================================== //

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

// ======================================== Base Publisher ======================================== //

///
/// @brief The Publisher class defines the publisher interface used by the Sample class to make it generic.
/// This allows any publisher specialization to be stored as a reference by the Sample class.
///
template<typename T>
class PublisherInterface
{
public:
    virtual void publish(Sample<T>& sample) noexcept = 0;
protected:
    PublisherInterface() = default;
};


///
/// @brief The BasePublisher class contains the common implementation for the different publisher specializations.
///
template<typename T, typename port_t = iox::popo::PublisherPortUser>
class BasePublisher : public PublisherInterface<T>
{
protected:

    BasePublisher(const BasePublisher& other) = delete;
    BasePublisher& operator=(const BasePublisher&) = delete;
    BasePublisher(BasePublisher&& rhs) = default;
    BasePublisher& operator=(BasePublisher&& rhs) = default;
    ~BasePublisher() = default;

    ///
    /// @brief uid Get the UID of the publisher.
    /// @return The publisher's UID.
    ///
    uid_t uid() const noexcept;

    ///
    /// @brief loan Get a sample from loaned shared memory.
    /// @param size The expected size of the sample.
    /// @return An instance of the sample that resides in shared memory or an error if unable ot allocate memory to laon.
    /// @details The loaned sample is automatically released when it goes out of scope.
    ///
    cxx::expected<Sample<T>, AllocationError> loan(uint32_t size) noexcept;

    ///
    /// @brief publish Publish the given sample.
    /// @param sample The sample to publish.
    ///
    void publish(Sample<T>& sample) noexcept override;

    ///
    /// @brief previousSample Retrieve the previously loaned sample if it has not yet been claimed.
    /// @return The previously loaned sample if retrieved.
    ///
    cxx::optional<Sample<T>> previousSample() noexcept;

    ///
    /// @brief offer Offer the service to be subscribed to.
    ///
    void offer() noexcept;

    ///
    /// @brief stopOffer Stop offering the service.
    ///
    void stopOffer() noexcept;

    ///
    /// @brief isOffered
    /// @return True if service is currently being offered.
    ///
    bool isOffered() noexcept;

    ///
    /// @brief hasSubscribers
    /// @return True if currently has subscribers to the service.
    ///
    bool hasSubscribers() noexcept;

protected:
    BasePublisher(const capro::ServiceDescription& service);

private:
    ///
    /// @brief convertChunkHeaderToSample Helper function that wraps the payload of a ChunkHeader in an Sample.
    /// @param header The chunk header describing the allocated memory chunk to use in the sample.
    /// @return A sample that uses the ChunkHeader's payload as its memory allocation.
    ///
    Sample<T> convertChunkHeaderToSample(const mepoo::ChunkHeader* header) noexcept;

protected:

    port_t m_port{nullptr};
    bool m_useDynamicPayloadSize = true;

};

// ======================================== Typed Publisher ======================================== //

template<typename T, typename base_publisher_t = iox::popo::BasePublisher<T>>
class TypedPublisher : public base_publisher_t
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
    void publish(Sample<T>& sample) noexcept;
    ///
    /// @brief publishCopyOf Copy the provided value into a loaned shared memory chunk and publish it.
    /// @param val Value to copy.
    /// @return Error if unable to allocate memory to loan.
    ///
    cxx::expected<AllocationError> publishCopyOf(const T& val) noexcept;

    ///
    /// @brief publishResultOf Loan a sample from memory, execute the provided callable to write to it, then publish it.
    /// @param c Callable with the signature void(T*, ArgTypes...) that write's it's result to T*.
    /// @param args The arguments of the callable.
    /// @return Error if unable to allocate memory to loan.
    ///
    template<typename Callable, typename... ArgTypes>
    cxx::expected<AllocationError> publishResultOf(Callable c, ArgTypes... args) noexcept;

    cxx::optional<Sample<T>> previousSample() noexcept;

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;
};

// ======================================== Untyped Publisher ======================================== //

template<typename base_publisher_t = iox::popo::BasePublisher<void>>
class UntypedPublisher : public base_publisher_t
{
public:
    UntypedPublisher(const capro::ServiceDescription& service);
    UntypedPublisher(const UntypedPublisher& other) = delete;
    UntypedPublisher& operator=(const UntypedPublisher&) = delete;
    UntypedPublisher(UntypedPublisher&& rhs) = default;
    UntypedPublisher& operator=(UntypedPublisher&& rhs) = default;
    ~UntypedPublisher() = default;

    uid_t uid() const noexcept;

    cxx::expected<Sample<void>, AllocationError> loan(uint32_t size) noexcept;
    void publish(Sample<void>& sample) noexcept;
    ///
    /// @brief publish Publish the provided memory chunk.
    /// @param allocatedMemory Pointer to the allocated shared memory chunk.
    /// @return Error if provided pointer is not a valid memory chunk.
    ///
    void publish(void* allocatedMemory) noexcept;
    cxx::optional<Sample<void>> previousSample() noexcept;

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;
};

}
}

#include "iceoryx_posh/experimental/internal/popo/publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP
