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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_SUBSCRIBER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_SUBSCRIBER_INL

namespace iox {
namespace popo {

UntypedSubscriber::UntypedSubscriber(const capro::ServiceDescription& service)
    : BaseSubscriber<void>(service)
{}

inline cxx::expected<SubscriberError>
UntypedSubscriber::subscribe(const uint64_t queueCapacity) noexcept
{
    return BaseSubscriber<void>::subscribe(queueCapacity);
}

inline SubscribeState
UntypedSubscriber::getSubscriptionState() const noexcept
{
    return BaseSubscriber<void>::getSubscriptionState();
}

inline void
UntypedSubscriber::unsubscribe() noexcept
{
    return BaseSubscriber<void>::unsubscribe();
}

inline bool
UntypedSubscriber::hasData() const noexcept
{
    return BaseSubscriber<void>::hasData();
}

inline cxx::expected<cxx::optional<cxx::unique_ptr<void>>>
UntypedSubscriber::receive() noexcept
{
    return BaseSubscriber<void>::receive();
}

inline cxx::optional<cxx::unique_ptr<mepoo::ChunkHeader>>
UntypedSubscriber::receiveHeader() noexcept
{
    return BaseSubscriber<void>::receiveHeader();
}

inline void
UntypedSubscriber::clearReceiveBuffer() noexcept
{
    BaseSubscriber<void>::clearReceiveBuffer();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_UNTYPED_SUBSCRIBER_INL
