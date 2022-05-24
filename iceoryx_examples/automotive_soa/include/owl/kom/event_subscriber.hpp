// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_SUBSCRIBER_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_SUBSCRIBER_HPP

#include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

#include "owl/types.hpp"

namespace owl
{
namespace kom
{
/// @note Once a receive handler has been set, calling the following methods not be thread-safe:
///           - Subscribe()
///           - Unsubscribe()
///           - GetNewSamples()
template <typename T>
class EventSubscriber
{
  public:
    using SampleType = T;

    static constexpr uint64_t HISTORY_REQUEST{1U};
    static constexpr bool NOT_OFFERED_ON_CREATE{false};

    EventSubscriber(const core::String& service, const core::String& instance, const core::String& event) noexcept;

    /// @note Will disable the receive handler if active
    void Subscribe(std::size_t) noexcept;
    /// @note Will disable the receive handler if active
    void Unsubscribe() noexcept;

    template <typename Callable>
    core::Result<size_t> GetNewSamples(Callable&& callable,
                                       size_t maxNumberOfSamples = std::numeric_limits<size_t>::max()) noexcept;

    void SetReceiveHandler(EventReceiveHandler handler) noexcept;
    void UnsetReceiveHandler() noexcept;
    bool HasReceiveHandler() noexcept;


  private:
    static void onSampleReceivedCallback(iox::popo::Subscriber<T>*, EventSubscriber* self) noexcept;

    //! [EventSubscriber members]
    iox::capro::ServiceDescription m_serviceDescription;
    iox::cxx::optional<iox::popo::Subscriber<T>> m_subscriber;
    iox::concurrent::smart_lock<iox::cxx::optional<iox::cxx::function<void()>>> m_receiveHandler;
    iox::popo::Listener m_listener;
    //! [EventSubscriber members]
};

} // namespace kom
} // namespace owl

#include "owl/kom/event_subscriber.inl"

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_SUBSCRIBER_HPP
