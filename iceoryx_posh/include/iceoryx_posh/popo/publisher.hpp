// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_PUBLISHER_HPP
#define IOX_POSH_POPO_TYPED_PUBLISHER_HPP

#include "iceoryx_posh/popo/base_publisher.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_utils/cxx/type_traits.hpp"

namespace iox
{
namespace popo
{
///
/// @brief The PublisherInterface class defines the publisher interface used by the Sample class to make it generic.
/// This allows any publisher specialization to be stored as a reference by the Sample class.
/// It is also needed to avoid circular dependencies between Sample and Publisher.
///
template <typename T>
class PublisherInterface
{
  public:
    virtual void publish(Sample<T>&& sample) noexcept = 0;

    virtual ~PublisherInterface(){};

  protected:
    PublisherInterface() = default;
};
template <typename T, typename base_publisher_t = BasePublisher<>>
class Publisher : public base_publisher_t, public PublisherInterface<T>
{
    static_assert(!std::is_void<T>::value, "Type must not be void. Use the UntypedPublisher for void types.");
    static_assert(std::is_default_constructible<T>::value, "The Publisher requires default-constructable types.");

  public:
    Publisher(const capro::ServiceDescription& service, const PublisherOptions& publisherOptions = PublisherOptions());
    Publisher(const Publisher& other) = delete;
    Publisher& operator=(const Publisher&) = delete;
    Publisher(Publisher&& rhs) = default;
    Publisher& operator=(Publisher&& rhs) = default;
    virtual ~Publisher() = default;

    ///
    /// @brief loan Get a sample from loaned shared memory and consctruct the data with the given arguments.
    /// @param args Arguments used to construct the data.
    /// @return An instance of the sample that resides in shared memory or an error if unable ot allocate memory to
    /// loan.
    /// @details The loaned sample is automatically released when it goes out of scope.
    ///
    template <typename... Args>
    cxx::expected<Sample<T>, AllocationError> loan(Args&&... args) noexcept;

    ///
    /// @brief previousSample Retrieve the previously loaned sample if it has not yet been claimed.
    /// @return The previously loaned sample if retrieved.
    ///
    cxx::optional<Sample<T>> loanPreviousSample() noexcept;

    ///
    /// @brief publish Publishes the given sample and then releases its loan.
    /// @param sample The sample to publish.
    ///
    void publish(Sample<T>&& sample) noexcept override;

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
    template <typename Callable, typename... ArgTypes>
    cxx::expected<AllocationError> publishResultOf(Callable c, ArgTypes... args) noexcept;

  protected:
    using base_publisher_t::port;

  private:
    Sample<T> convertChunkHeaderToSample(const mepoo::ChunkHeader* const header) noexcept;

    cxx::expected<Sample<T>, AllocationError> loanSample() noexcept;

    using PublisherSampleDeleter = SampleDeleter<typename base_publisher_t::PortType>;
    PublisherSampleDeleter m_sampleDeleter{port()};
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/publisher.inl"

#endif // IOX_POSH_POPO_TYPED_PUBLISHER_HPP
