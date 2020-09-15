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

#include "iceoryx_posh/experimental/popo/sample.hpp"
#include "iceoryx_posh/experimental/popo/base_publisher.hpp"

namespace iox {
namespace popo {

template<typename base_publisher_t = BasePublisher<void>>
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

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/untyped_publisher.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_PUBLISHER_HPP
