// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by Michael Bentley <mikebentley15@gmail.com>. All rights reserved.
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

#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_binding_c/internal/cpp2c_waitset.hpp"
#include "iceoryx_posh/popo/notification_callback.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iox/assertions.hpp"

#include <type_traits>

using namespace iox;
using namespace iox::popo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/wait_set.h"
}

static uint64_t notification_info_vector_to_c_array(const WaitSet<>::NotificationInfoVector& triggerVector,
                                                    iox_notification_info_t* notificationInfoArray,
                                                    const uint64_t notificationInfoArrayCapacity,
                                                    uint64_t* missedElements)
{
    uint64_t notificationInfoArraySize = 0U;
    uint64_t triggerVectorSize = triggerVector.size();
    if (triggerVectorSize > notificationInfoArrayCapacity)
    {
        *missedElements = triggerVectorSize - notificationInfoArrayCapacity;
        notificationInfoArraySize = notificationInfoArrayCapacity;
    }
    else
    {
        *missedElements = 0U;
        notificationInfoArraySize = triggerVectorSize;
    }

    for (uint64_t i = 0U; i < notificationInfoArraySize; ++i)
    {
        notificationInfoArray[i] = triggerVector[i];
    }

    return notificationInfoArraySize;
}

iox_ws_t iox_ws_init(iox_ws_storage_t* self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    auto* me = new cpp2c_WaitSet();
    self->do_not_touch_me[0] = reinterpret_cast<uint64_t>(me);
    return me;
}

void iox_ws_deinit(iox_ws_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    delete self;
}

uint64_t iox_ws_timed_wait(iox_ws_t const self,
                           struct timespec timeout,
                           iox_notification_info_t* const notificationInfoArray,
                           const uint64_t notificationInfoArrayCapacity,
                           uint64_t* missedElements)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(missedElements != nullptr, "'missedElements' must not be a 'nullptr'");

    return notification_info_vector_to_c_array(self->timedWait(units::Duration(timeout)),
                                               notificationInfoArray,
                                               notificationInfoArrayCapacity,
                                               missedElements);
}

uint64_t iox_ws_wait(iox_ws_t const self,
                     iox_notification_info_t* const notificationInfoArray,
                     const uint64_t notificationInfoArrayCapacity,
                     uint64_t* missedElements)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(missedElements != nullptr, "'missedElements' must not be a 'nullptr'");

    return notification_info_vector_to_c_array(
        self->wait(), notificationInfoArray, notificationInfoArrayCapacity, missedElements);
}

uint64_t iox_ws_size(iox_ws_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->size();
}

uint64_t iox_ws_capacity(iox_ws_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    return self->capacity();
}

void iox_ws_mark_for_destruction(iox_ws_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    self->markForDestruction();
}

iox_WaitSetResult iox_ws_attach_subscriber_state(iox_ws_t const self,
                                                 iox_sub_t const subscriber,
                                                 const iox_SubscriberState subscriberState,
                                                 const uint64_t eventId,
                                                 void (*callback)(iox_sub_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");

    auto result = self->attachState(*subscriber, c2cpp::subscriberState(subscriberState), eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_subscriber_state_with_context_data(iox_ws_t const self,
                                                                   iox_sub_t const subscriber,
                                                                   const iox_SubscriberState subscriberState,
                                                                   const uint64_t eventId,
                                                                   void (*callback)(iox_sub_t, void*),
                                                                   void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");

    NotificationCallback<cpp2c_Subscriber, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result =
        self->attachState(*subscriber, c2cpp::subscriberState(subscriberState), eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_subscriber_event(iox_ws_t const self,
                                                 iox_sub_t const subscriber,
                                                 const iox_SubscriberEvent subscriberEvent,
                                                 const uint64_t eventId,
                                                 void (*callback)(iox_sub_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");

    auto result = self->attachEvent(*subscriber, c2cpp::subscriberEvent(subscriberEvent), eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_subscriber_event_with_context_data(iox_ws_t const self,
                                                                   iox_sub_t const subscriber,
                                                                   const iox_SubscriberEvent subscriberEvent,
                                                                   const uint64_t eventId,
                                                                   void (*callback)(iox_sub_t, void*),
                                                                   void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");

    NotificationCallback<cpp2c_Subscriber, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result =
        self->attachEvent(*subscriber, c2cpp::subscriberEvent(subscriberEvent), eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_user_trigger_event(iox_ws_t const self,
                                                   iox_user_trigger_t const userTrigger,
                                                   const uint64_t eventId,
                                                   void (*callback)(iox_user_trigger_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userTrigger != nullptr, "'userTrigger' must not be a 'nullptr'");

    auto result = self->attachEvent(*userTrigger, eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_user_trigger_event_with_context_data(iox_ws_t const self,
                                                                     iox_user_trigger_t const userTrigger,
                                                                     const uint64_t eventId,
                                                                     void (*callback)(iox_user_trigger_t, void*),
                                                                     void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userTrigger != nullptr, "'userTrigger' must not be a 'nullptr'");

    NotificationCallback<UserTrigger, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result = self->attachEvent(*userTrigger, eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

void iox_ws_detach_subscriber_event(iox_ws_t const self,
                                    iox_sub_t const subscriber,
                                    const iox_SubscriberEvent subscriberEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");

    self->detachEvent(*subscriber, c2cpp::subscriberEvent(subscriberEvent));
}

void iox_ws_detach_subscriber_state(iox_ws_t const self,
                                    iox_sub_t const subscriber,
                                    const iox_SubscriberState subscriberState)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");

    self->detachState(*subscriber, c2cpp::subscriberState(subscriberState));
}

void iox_ws_detach_user_trigger_event(iox_ws_t const self, iox_user_trigger_t const userTrigger)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userTrigger != nullptr, "'userTrigger' must not be a 'nullptr'");

    self->detachEvent(*userTrigger);
}

iox_WaitSetResult iox_ws_attach_client_event(const iox_ws_t self,
                                             const iox_client_t client,
                                             const iox_ClientEvent clientEvent,
                                             const uint64_t eventId,
                                             void (*callback)(iox_client_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");

    auto result = self->attachEvent(*client, c2cpp::clientEvent(clientEvent), eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_client_event_with_context_data(iox_ws_t const self,
                                                               iox_client_t const client,
                                                               const enum iox_ClientEvent clientEvent,
                                                               const uint64_t eventId,
                                                               void (*callback)(iox_client_t, void*),
                                                               void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");

    NotificationCallback<std::remove_pointer_t<iox_client_t>, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result = self->attachEvent(*client, c2cpp::clientEvent(clientEvent), eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_client_state(const iox_ws_t self,
                                             const iox_client_t client,
                                             const iox_ClientState clientState,
                                             const uint64_t eventId,
                                             void (*callback)(iox_client_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");

    auto result = self->attachState(*client, c2cpp::clientState(clientState), eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_client_state_with_context_data(iox_ws_t const self,
                                                               iox_client_t const client,
                                                               const enum iox_ClientState clientState,
                                                               const uint64_t eventId,
                                                               void (*callback)(iox_client_t, void*),
                                                               void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");

    NotificationCallback<std::remove_pointer_t<iox_client_t>, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result = self->attachState(*client, c2cpp::clientState(clientState), eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

void iox_ws_detach_client_event(iox_ws_t const self, iox_client_t const client, const enum iox_ClientEvent clientEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");

    self->detachEvent(*client, c2cpp::clientEvent(clientEvent));
}

void iox_ws_detach_client_state(iox_ws_t const self, iox_client_t const client, const enum iox_ClientState clientState)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");

    self->detachState(*client, c2cpp::clientState(clientState));
}

iox_WaitSetResult iox_ws_attach_server_event(const iox_ws_t self,
                                             const iox_server_t server,
                                             const enum iox_ServerEvent serverEvent,
                                             const uint64_t eventId,
                                             void (*callback)(iox_server_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");

    auto result = self->attachEvent(*server, c2cpp::serverEvent(serverEvent), eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_server_event_with_context_data(iox_ws_t const self,
                                                               iox_server_t const server,
                                                               const enum iox_ServerEvent serverEvent,
                                                               const uint64_t eventId,
                                                               void (*callback)(iox_server_t, void*),
                                                               void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");

    NotificationCallback<std::remove_pointer_t<iox_server_t>, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result = self->attachEvent(*server, c2cpp::serverEvent(serverEvent), eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_server_state(const iox_ws_t self,
                                             const iox_server_t server,
                                             const enum iox_ServerState serverState,
                                             const uint64_t eventId,
                                             void (*callback)(iox_server_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");

    auto result = self->attachState(*server, c2cpp::serverState(serverState), eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult iox_ws_attach_server_state_with_context_data(iox_ws_t const self,
                                                               iox_server_t const server,
                                                               const enum iox_ServerState serverState,
                                                               const uint64_t eventId,
                                                               void (*callback)(iox_server_t, void*),
                                                               void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");

    NotificationCallback<std::remove_pointer_t<iox_server_t>, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result = self->attachState(*server, c2cpp::serverState(serverState), eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

void iox_ws_detach_server_event(iox_ws_t const self, iox_server_t const server, const enum iox_ServerEvent serverEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");

    self->detachEvent(*server, c2cpp::serverEvent(serverEvent));
}

void iox_ws_detach_server_state(iox_ws_t const self, iox_server_t const server, const enum iox_ServerState serverState)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");

    self->detachState(*server, c2cpp::serverState(serverState));
}

iox_WaitSetResult iox_ws_attach_service_discovery_event(const iox_ws_t self,
                                                        const iox_service_discovery_t serviceDiscovery,
                                                        const enum iox_ServiceDiscoveryEvent serviceDiscoveryEvent,
                                                        const uint64_t eventId,
                                                        void (*callback)(iox_service_discovery_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceDiscovery != nullptr, "'serviceDiscovery' must not be a 'nullptr'");

    auto result = self->attachEvent(
        *serviceDiscovery, c2cpp::serviceDiscoveryEvent(serviceDiscoveryEvent), eventId, {callback, nullptr});
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

iox_WaitSetResult
iox_ws_attach_service_discovery_event_with_context_data(iox_ws_t const self,
                                                        iox_service_discovery_t const serviceDiscovery,
                                                        const enum iox_ServiceDiscoveryEvent serviceDiscoveryEvent,
                                                        const uint64_t eventId,
                                                        void (*callback)(iox_service_discovery_t, void*),
                                                        void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceDiscovery != nullptr, "'serviceDiscovery' must not be a 'nullptr'");

    NotificationCallback<std::remove_pointer_t<iox_service_discovery_t>, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result = self->attachEvent(
        *serviceDiscovery, c2cpp::serviceDiscoveryEvent(serviceDiscoveryEvent), eventId, notificationCallback);
    return (result.has_error()) ? cpp2c::waitSetResult(result.error()) : iox_WaitSetResult::WaitSetResult_SUCCESS;
}

void iox_ws_detach_service_discovery_event(iox_ws_t const self,
                                           iox_service_discovery_t const serviceDiscovery,
                                           const enum iox_ServiceDiscoveryEvent serviceDiscoveryEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceDiscovery != nullptr, "'serviceDiscovery' must not be a 'nullptr'");

    self->detachEvent(*serviceDiscovery, c2cpp::serviceDiscoveryEvent(serviceDiscoveryEvent));
}
