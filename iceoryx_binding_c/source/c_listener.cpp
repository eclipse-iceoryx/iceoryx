// Copyright (c) 2021 - 2022 Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iox/assertions.hpp"

#include <type_traits>

using namespace iox;
using namespace iox::popo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/listener.h"
}

iox_listener_t iox_listener_init(iox_listener_storage_t* self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    auto* me = new Listener();
    self->do_not_touch_me[0] = reinterpret_cast<uint64_t>(me);
    return me;
}

void iox_listener_deinit(iox_listener_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    delete self;
}

enum iox_ListenerResult iox_listener_attach_subscriber_event(iox_listener_t const self,
                                                             iox_sub_t const subscriber,
                                                             const enum iox_SubscriberEvent subscriberEvent,
                                                             void (*callback)(iox_sub_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");

    auto result =
        self->attachEvent(*subscriber,
                          c2cpp::subscriberEvent(subscriberEvent),
                          NotificationCallback<cpp2c_Subscriber, popo::internal::NoType_t>{callback, nullptr});
    if (result.has_error())
    {
        return cpp2c::listenerResult(result.error());
    }
    return ListenerResult_SUCCESS;
}

enum iox_ListenerResult
iox_listener_attach_subscriber_event_with_context_data(iox_listener_t const self,
                                                       iox_sub_t const subscriber,
                                                       const enum iox_SubscriberEvent subscriberEvent,
                                                       void (*callback)(iox_sub_t, void*),
                                                       void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");
    IOX_ENFORCE(contextData != nullptr, "'contextData' must not be a 'nullptr'");

    auto result = self->attachEvent(*subscriber,
                                    c2cpp::subscriberEvent(subscriberEvent),
                                    NotificationCallback<cpp2c_Subscriber, void>{callback, contextData});
    if (result.has_error())
    {
        return cpp2c::listenerResult(result.error());
    }
    return ListenerResult_SUCCESS;
}

enum iox_ListenerResult iox_listener_attach_user_trigger_event(iox_listener_t const self,
                                                               iox_user_trigger_t const userTrigger,
                                                               void (*callback)(iox_user_trigger_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userTrigger != nullptr, "'userTrigger' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");

    auto result =
        self->attachEvent(*userTrigger, NotificationCallback<UserTrigger, popo::internal::NoType_t>{callback, nullptr});
    if (result.has_error())
    {
        return cpp2c::listenerResult(result.error());
    }
    return ListenerResult_SUCCESS;
}

enum iox_ListenerResult iox_listener_attach_user_trigger_event_with_context_data(iox_listener_t const self,
                                                                                 iox_user_trigger_t const userTrigger,
                                                                                 void (*callback)(iox_user_trigger_t,
                                                                                                  void*),
                                                                                 void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userTrigger != nullptr, "'userTrigger' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");
    IOX_ENFORCE(contextData != nullptr, "'contextData' must not be a 'nullptr'");

    NotificationCallback<UserTrigger, void> notificationCallback;
    notificationCallback.m_callback = callback;
    notificationCallback.m_contextData = contextData;

    auto result = self->attachEvent(*userTrigger, NotificationCallback<UserTrigger, void>{callback, contextData});
    if (result.has_error())
    {
        return cpp2c::listenerResult(result.error());
    }
    return ListenerResult_SUCCESS;
}

void iox_listener_detach_subscriber_event(iox_listener_t const self,
                                          iox_sub_t const subscriber,
                                          const enum iox_SubscriberEvent subscriberEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(subscriber != nullptr, "'subscriver' must not be a 'nullptr'");

    self->detachEvent(*subscriber, c2cpp::subscriberEvent(subscriberEvent));
}

void iox_listener_detach_user_trigger_event(iox_listener_t const self, iox_user_trigger_t const userTrigger)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(userTrigger != nullptr, "'userTrigger' must not be a 'nullptr'");

    self->detachEvent(*userTrigger);
}

uint64_t iox_listener_size(iox_listener_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    return self->size();
}

uint64_t iox_listener_capacity(iox_listener_t const self)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");

    return self->capacity();
}

iox_ListenerResult iox_listener_attach_client_event(iox_listener_t const self,
                                                    iox_client_t const client,
                                                    const enum iox_ClientEvent clientEvent,
                                                    void (*callback)(iox_client_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");

    auto result = self->attachEvent(
        *client,
        c2cpp::clientEvent(clientEvent),
        NotificationCallback<std::remove_pointer_t<iox_client_t>, popo::internal::NoType_t>{callback, nullptr});
    return (result.has_error()) ? cpp2c::listenerResult(result.error()) : iox_ListenerResult::ListenerResult_SUCCESS;
}

iox_ListenerResult iox_listener_attach_client_event_with_context_data(iox_listener_t const self,
                                                                      iox_client_t const client,
                                                                      const enum iox_ClientEvent clientEvent,
                                                                      void (*callback)(iox_client_t, void*),
                                                                      void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");
    IOX_ENFORCE(contextData != nullptr, "'contextData' must not be a 'nullptr'");

    auto result =
        self->attachEvent(*client,
                          c2cpp::clientEvent(clientEvent),
                          NotificationCallback<std::remove_pointer_t<iox_client_t>, void>{callback, contextData});
    return (result.has_error()) ? cpp2c::listenerResult(result.error()) : iox_ListenerResult::ListenerResult_SUCCESS;
}

void iox_listener_detach_client_event(iox_listener_t const self,
                                      iox_client_t const client,
                                      const enum iox_ClientEvent clientEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(client != nullptr, "'client' must not be a 'nullptr'");

    self->detachEvent(*client, c2cpp::clientEvent(clientEvent));
}


iox_ListenerResult iox_listener_attach_server_event(iox_listener_t const self,
                                                    iox_server_t const server,
                                                    const enum iox_ServerEvent serverEvent,
                                                    void (*callback)(iox_server_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");

    auto result = self->attachEvent(
        *server,
        c2cpp::serverEvent(serverEvent),
        NotificationCallback<std::remove_pointer_t<iox_server_t>, popo::internal::NoType_t>{callback, nullptr});
    return (result.has_error()) ? cpp2c::listenerResult(result.error()) : iox_ListenerResult::ListenerResult_SUCCESS;
}

iox_ListenerResult iox_listener_attach_server_event_with_context_data(iox_listener_t const self,
                                                                      iox_server_t const server,
                                                                      const enum iox_ServerEvent serverEvent,
                                                                      void (*callback)(iox_server_t, void*),
                                                                      void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");
    IOX_ENFORCE(contextData != nullptr, "'contextData' must not be a 'nullptr'");

    auto result =
        self->attachEvent(*server,
                          c2cpp::serverEvent(serverEvent),
                          NotificationCallback<std::remove_pointer_t<iox_server_t>, void>{callback, contextData});
    return (result.has_error()) ? cpp2c::listenerResult(result.error()) : iox_ListenerResult::ListenerResult_SUCCESS;
}

void iox_listener_detach_server_event(iox_listener_t const self,
                                      iox_server_t const server,
                                      const enum iox_ServerEvent serverEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(server != nullptr, "'server' must not be a 'nullptr'");

    self->detachEvent(*server, c2cpp::serverEvent(serverEvent));
}

iox_ListenerResult
iox_listener_attach_service_discovery_event(iox_listener_t const self,
                                            iox_service_discovery_t const serviceDiscovery,
                                            const enum iox_ServiceDiscoveryEvent serviceDiscoveryEvent,
                                            void (*callback)(iox_service_discovery_t))
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceDiscovery != nullptr, "'serviceDiscovery' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");

    auto result =
        self->attachEvent(*serviceDiscovery,
                          c2cpp::serviceDiscoveryEvent(serviceDiscoveryEvent),
                          NotificationCallback<ServiceDiscovery, popo::internal::NoType_t>{callback, nullptr});

    return (result.has_error()) ? cpp2c::listenerResult(result.error()) : iox_ListenerResult::ListenerResult_SUCCESS;
}

iox_ListenerResult iox_listener_attach_service_discovery_event_with_context_data(
    iox_listener_t const self,
    iox_service_discovery_t const serviceDiscovery,
    const enum iox_ServiceDiscoveryEvent serviceDiscoveryEvent,
    void (*callback)(iox_service_discovery_t, void*),
    void* const contextData)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceDiscovery != nullptr, "'serviceDiscovery' must not be a 'nullptr'");
    IOX_ENFORCE(callback != nullptr, "'callback' must not be a 'nullptr'");
    IOX_ENFORCE(contextData != nullptr, "'contextData' must not be a 'nullptr'");

    auto result = self->attachEvent(*serviceDiscovery,
                                    c2cpp::serviceDiscoveryEvent(serviceDiscoveryEvent),
                                    NotificationCallback<ServiceDiscovery, void>{callback, contextData});

    return (result.has_error()) ? cpp2c::listenerResult(result.error()) : iox_ListenerResult::ListenerResult_SUCCESS;
}

void iox_listener_detach_service_discovery_event(iox_listener_t const self,
                                                 iox_service_discovery_t const serviceDiscovery,
                                                 const enum iox_ServiceDiscoveryEvent serviceDiscoveryEvent)
{
    IOX_ENFORCE(self != nullptr, "'self' must not be a 'nullptr'");
    IOX_ENFORCE(serviceDiscovery != nullptr, "'serviceDiscovery' must not be a 'nullptr'");

    self->detachEvent(*serviceDiscovery, c2cpp::serviceDiscoveryEvent(serviceDiscoveryEvent));
}
