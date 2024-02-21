// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_PUBLISHER_IMPL_HPP
#define IOX_POSH_POPO_TYPED_PUBLISHER_IMPL_HPP

#include "iceoryx_posh/internal/popo/base_publisher.hpp"
#include "iceoryx_posh/internal/popo/publisher_interface.hpp"
#include "iceoryx_posh/internal/popo/typed_port_api_trait.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iox/type_traits.hpp"

namespace iox
{
namespace popo
{
/// @brief The PublisherImpl class implements the typed publisher API
/// @note Not intended for public usage! Use the 'Publisher' instead!
template <typename T, typename H = mepoo::NoUserHeader, typename BasePublisherType = BasePublisher<>>
class PublisherImpl : public BasePublisherType, private PublisherInterface<T, H>
{
    using DataTypeAssert = typename TypedPortApiTrait<T>::Assert;
    using HeaderTypeAssert = typename TypedPortApiTrait<H>::Assert;

  public:
    explicit PublisherImpl(const capro::ServiceDescription& service,
                           const PublisherOptions& publisherOptions = PublisherOptions());

    virtual ~PublisherImpl() = default;

    PublisherImpl(const PublisherImpl& other) = delete;
    PublisherImpl& operator=(const PublisherImpl&) = delete;
    PublisherImpl(PublisherImpl&& rhs) noexcept = delete;
    PublisherImpl& operator=(PublisherImpl&& rhs) noexcept = delete;

    ///
    /// @brief loan Get a sample from loaned shared memory and consctruct the data with the given arguments.
    /// @param args Arguments used to construct the data.
    /// @return An instance of the sample that resides in shared memory or an error if unable ot allocate memory to
    /// loan.
    /// @details The loaned sample is automatically released when it goes out of scope.
    ///
    template <typename... Args>
    expected<Sample<T, H>, AllocationError> loan(Args&&... args) noexcept;

    ///
    /// @brief publish Publishes the given sample and then releases its loan.
    /// @param sample The sample to publish.
    ///
    void publish(Sample<T, H>&& sample) noexcept override;

    ///
    /// @brief publishCopyOf Copy the provided value into a loaned shared memory chunk and publish it.
    /// @param val Value to copy.
    /// @return Error if unable to allocate memory to loan.
    ///
    expected<void, AllocationError> publishCopyOf(const T& val) noexcept;
    ///
    /// @brief publishResultOf Loan a sample from memory, execute the provided callable to write to it, then publish it.
    /// @param c Callable with the signature void(T*, ArgTypes...) that write's it's result to T*.
    /// @param args The arguments of the callable.
    /// @return Error if unable to allocate memory to loan.
    ///
    template <typename Callable, typename... ArgTypes>
    expected<void, AllocationError> publishResultOf(Callable c, ArgTypes... args) noexcept;

  protected:
    using PortType = typename BasePublisherType::PortType;
    using BasePublisherType::port;

    PublisherImpl(PortType&& port) noexcept;

  private:
    Sample<T, H> convertChunkHeaderToSample(mepoo::ChunkHeader* const header) noexcept;

    expected<Sample<T, H>, AllocationError> loanSample() noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/publisher_impl.inl"

#endif // IOX_POSH_POPO_TYPED_PUBLISHER_IMPL_HPP
