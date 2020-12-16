// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_UNTYPED_PUBLISHER_HPP
#define IOX_POSH_POPO_UNTYPED_PUBLISHER_HPP

#include "iceoryx_posh/popo/base_publisher.hpp"
#include "iceoryx_posh/popo/sample.hpp"

namespace iox
{
namespace popo
{
template <typename base_publisher_t = BasePublisher<void>>
class UntypedPublisherImpl : public base_publisher_t
{
  public:
    UntypedPublisherImpl(const capro::ServiceDescription& service,
                         const PublisherOptions& publisherOptions = PublisherOptions());
    UntypedPublisherImpl(const UntypedPublisherImpl& other) = delete;
    UntypedPublisherImpl& operator=(const UntypedPublisherImpl&) = delete;
    UntypedPublisherImpl(UntypedPublisherImpl&& rhs) = default;
    UntypedPublisherImpl& operator=(UntypedPublisherImpl&& rhs) = default;
    virtual ~UntypedPublisherImpl() = default;

    using base_publisher_t::getServiceDescription;
    using base_publisher_t::getUid;
    using base_publisher_t::hasSubscribers;
    using base_publisher_t::isOffered;
    using base_publisher_t::loan;
    using base_publisher_t::loanPreviousSample;
    using base_publisher_t::offer;
    using base_publisher_t::publish;
    using base_publisher_t::stopOffer;

    ///
    /// @brief publish Publish the provided memory chunk.
    /// @param allocatedMemory Pointer to the allocated shared memory chunk.
    /// @return Error if provided pointer is not a valid memory chunk.
    ///
    void publish(void* allocatedMemory) noexcept;
};

using UntypedPublisher = UntypedPublisherImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_publisher.inl"

#endif // IOX_POSH_POPO_UNTYPED_PUBLISHER_HPP
