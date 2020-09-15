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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_SUBSCRIBER_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/experimental/popo/base_subscriber.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox {
namespace popo {

class UntypedSubscriber : protected BaseSubscriber<void>
{
public:

    UntypedSubscriber(const capro::ServiceDescription& service);
    UntypedSubscriber(const UntypedSubscriber& other) = delete;
    UntypedSubscriber& operator=(const UntypedSubscriber&) = delete;
    UntypedSubscriber(UntypedSubscriber&& rhs) = delete;
    UntypedSubscriber& operator=(UntypedSubscriber&& rhs) = delete;
    ~UntypedSubscriber() = default;

    capro::ServiceDescription getServiceDescription() const noexcept;
    uid_t uid() const noexcept;

    cxx::expected<SubscriberError> subscribe(const uint64_t queueCapacity = MAX_SUBSCRIBER_QUEUE_CAPACITY) noexcept;
    SubscribeState getSubscriptionState() const noexcept;
    void unsubscribe() noexcept;

    bool hasData() const noexcept;
    cxx::optional<cxx::unique_ptr<void>> receive() noexcept;
    cxx::optional<cxx::unique_ptr<mepoo::ChunkHeader>> receiveHeader() noexcept;
    void clearReceiveBuffer() noexcept;

};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/untyped_subscriber.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_SUBSCRIBER_HPP
