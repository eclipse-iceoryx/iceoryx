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

#ifndef IOX_POSH_POPO_BASE_PUBLISHER_HPP
#define IOX_POSH_POPO_BASE_PUBLISHER_HPP

#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/popo/modern_api/sample.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

namespace iox
{
namespace popo
{
using uid_t = UniquePortId;

///
/// @brief The PublisherInterface class defines the publisher interface used by the Sample class to make it generic.
/// This allows any publisher specialization to be stored as a reference by the Sample class.
///
template <typename T>
class PublisherInterface
{
  public:
    virtual void publish(Sample<T>&& sample) noexcept = 0;

  protected:
    PublisherInterface() = default;
};

///
/// @brief The BasePublisher class contains the common implementation for the different publisher specializations.
///
template <typename T, typename port_t = iox::PublisherPortUserType>
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
    uid_t getUid() const noexcept;

    ///
    /// @brief getServiceDescription Get the service description of the publisher.
    /// @return The service description.
    ///
    capro::ServiceDescription getServiceDescription() const noexcept;

    ///
    /// @brief loan Get a sample from loaned shared memory.
    /// @param size The expected size of the sample.
    /// @return An instance of the sample that resides in shared memory or an error if unable ot allocate memory to
    /// laon.
    /// @details The loaned sample is automatically released when it goes out of scope.
    ///
    cxx::expected<Sample<T>, AllocationError> loan(const uint32_t size) noexcept;

    ///
    /// @brief publish Publishes the given sample and then releases its loan.
    /// @param sample The sample to publish.
    ///
    void publish(Sample<T>&& sample) noexcept override;

    ///
    /// @brief previousSample Retrieve the previously loaned sample if it has not yet been claimed.
    /// @return The previously loaned sample if retrieved.
    ///
    cxx::optional<Sample<T>> loanPreviousSample() noexcept;

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
    bool isOffered() const noexcept;

    ///
    /// @brief hasSubscribers
    /// @return True if currently has subscribers to the service.
    ///
    bool hasSubscribers() const noexcept;

  protected:
    BasePublisher() = default; // Required for testing.
    BasePublisher(const capro::ServiceDescription& service);

  private:
    ///
    /// @brief The PublisherSampleDeleter struct is a custom deleter in functor form which releases loans to a sample's
    /// underlying memory chunk via the publisher port.
    /// Each publisher should create its own instance of this deleter struct to work with its specific port.
    ///
    /// @note As this deleter is coupled to the Publisher implementation, it should only be used within the publisher
    /// context.
    ///
    struct PublisherSampleDeleter
    {
      public:
        PublisherSampleDeleter(port_t& port);
        void operator()(T* const ptr) const;

      private:
        std::reference_wrapper<port_t> m_port;
    };

    ///
    /// @brief convertChunkHeaderToSample Helper function that wraps the payload of a ChunkHeader in an Sample.
    /// @param header The chunk header describing the allocated memory chunk to use in the sample.
    /// @return A sample that uses the ChunkHeader's payload as its memory allocation.
    ///
    Sample<T> convertChunkHeaderToSample(const mepoo::ChunkHeader* const header) noexcept;

  protected:
    port_t m_port{nullptr};
    bool m_useDynamicPayloadSize = true;

    PublisherSampleDeleter m_sampleDeleter{m_port};
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/modern_api/base_publisher.inl"

#endif // IOX_POSH_POPO_BASE_PUBLISHER_HPP
