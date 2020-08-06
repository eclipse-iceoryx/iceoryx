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

#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"

#include <atomic>

namespace iox
{
namespace popo
{

template <typename T, typename port_t = iox::popo::PublisherPortUser>
class Publisher;

template <typename T>
class Sample
{
public:
    Sample(T* allocation, const cxx::function_ref<void(T* const)> deleter, Publisher<T>& publisher)
        : m_samplePtr(allocation, deleter), m_publisher(publisher) {}

    Sample(const Sample& other) = delete;
    Sample& operator=(const Sample&) = delete;
    Sample(Sample&& rhs) = default;
    Sample& operator=(Sample&& rhs) = default;

    T* operator->() noexcept
    {
        return m_samplePtr.get();
    }
    T* allocation() noexcept
    {
        return m_samplePtr.get();
    }
    template <typename... Args>
    void emplace(Args&&... args) noexcept
    {
        if(m_samplePtr != nullptr)
        {
            new (m_samplePtr.get()) T(std::forward<Args>(args)...);
            m_isEmpty = false;
        }
    }
    void emplace(cxx::function_ref<void(T*)> f)
    {
        f(m_samplePtr.get());
        m_isEmpty = false;
    }
    void publish() noexcept
    {
        m_publisher.publish(std::move(*this));
    }
private:
    const cxx::unique_ptr<T> m_samplePtr = nullptr;
    bool m_isEmpty = true;
    Publisher<T>& m_publisher;
};

enum class ChunkRecallError : uint8_t
{
    NO_PREVIOUS_CHUNK,
    CHUNK_ALREADY_CLAIMED
};

struct Untyped{};

template<typename T, typename port_t>
class Publisher
{
public:
    //using SamplePtr = cxx::unique_ptr<T>;
    using uid_t = uint64_t;

    // Temporary, to be replaced with service description / id based constructors
    Publisher() = default;

    ///
    /// @brief Publisher Create publisher for specified service [legacy].
    /// @param service Service to publish to.
    ///
    Publisher(const capro::ServiceDescription& service);

    Publisher(const Publisher& other) = delete;
    Publisher& operator=(const Publisher&) = delete;
    Publisher(Publisher&& rhs) = default;
    Publisher& operator=(Publisher&& rhs) = default;
    ~Publisher() = default;

    ///
    /// @brief uid Get the unique id of the publisher.
    /// @return The unique id.
    ///
    uid_t uid() const noexcept;

    ///
    /// @brief loan Loan an empty sample from the shared memory pool.
    /// @return Pointer to the successfully loaned sample, otherwise an allocation error.
    ///
    cxx::expected<Sample<T>, AllocationError> loan() noexcept;

//    ///
//    /// @brief loan  Loan an empty sample from the shared memory pool and process it with the given callable.
//    /// @param f Function to execute, taking the allocated chunk as its parameter.
//    /// @return Success if the loaned sample
//    ///
//    cxx::expected<AllocationError> loan(cxx::function_ref<void(Sample<T>&) noexcept> f) noexcept;

    ///
    /// @brief release Releases ownership of an unused allocated chunk.
    /// @details The released chunk will be freed as soon as there are no longer any active references
    /// to it in the system.
    /// @param chunk
    ///
    void release(Sample<T>&& sample) noexcept;

    ///
    /// @brief send Publishes the loaned sample to all subscribers.
    /// @details The loanded sample is automatically released after publishing.
    /// @param chunk
    ///
    void publish(Sample<T>&& sample) noexcept;

    ///
    /// @brief publish Publishes the value returned to the given function to all subscribers.
    /// @details Sample is automatically loaned.
    /// @param f Function that prodocues a value T at the provided location.
    ///
    void publish(cxx::function_ref<void(T*)> f) noexcept;

    ///
    /// @brief copyAndPublish Copy the given sample into a loaned sample and publish it to all subscribers.
    /// @details This method should not be used for larger data types as it includes a copy. For larger data types, it
    /// is preferred to first laon an empty sample and then directly write the data into it (e.g. with a placement new)
    /// rather than to write it elsewhere then copy it in.
    /// @param val The value to publish
    ///
    void publishCopyOf(const T& val) noexcept;

    ///
    /// @brief previous Reclaims ownership of a previously published sample if it has not yet been accessed by subscribers.
    /// @return The previously published sample if one exists and is unclaimed, otherwise an error.
    ///
    cxx::expected<ChunkRecallError> previous() const noexcept;

    void offer() const noexcept;
    void stopOffer() const noexcept;
    bool isOffered() const noexcept;
    bool hasSubscribers() const noexcept;

protected:
    port_t m_port{nullptr};
    bool m_useDynamicPayloadSize = true;

private:


};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_HPP
