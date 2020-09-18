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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_PUBLISHER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_PUBLISHER_HPP

#include "iceoryx_posh/experimental/popo/base_publisher.hpp"
#include "iceoryx_posh/experimental/popo/publishable_sample.hpp"

namespace iox
{
namespace popo
{
template <typename base_publisher_t = BasePublisher<void>>
class UntypedPublisherImpl : public base_publisher_t
{
  public:
    UntypedPublisherImpl(const capro::ServiceDescription& service);
    UntypedPublisherImpl(const UntypedPublisherImpl& other) = delete;
    UntypedPublisherImpl& operator=(const UntypedPublisherImpl&) = delete;
    UntypedPublisherImpl(UntypedPublisherImpl&& rhs) = default;
    UntypedPublisherImpl& operator=(UntypedPublisherImpl&& rhs) = default;
    ~UntypedPublisherImpl() = default;

    uid_t getUid() const noexcept;

    cxx::expected<Sample<void>, AllocationError> loan(uint32_t size) noexcept;
    void publish(Sample<void> sample) noexcept;
    ///
    /// @brief publish Publish the provided memory chunk.
    /// @param allocatedMemory Pointer to the allocated shared memory chunk.
    /// @return Error if provided pointer is not a valid memory chunk.
    ///
    void publish(void* allocatedMemory) noexcept;
    cxx::optional<Sample<void>> loanPreviousSample() noexcept;

    void offer() noexcept;
    void stopOffer() noexcept;
    bool isOffered() noexcept;
    bool hasSubscribers() noexcept;
};

using UntypedPublisher = UntypedPublisherImpl<>;

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/untyped_publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_PUBLISHER_HPP
