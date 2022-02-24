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

#ifndef IOX_BINDING_C_EVENT_INFO_H
#define IOX_BINDING_C_EVENT_INFO_H

#include "iceoryx_binding_c/client.h"
#include "iceoryx_binding_c/internal/c2cpp_binding.h"
#include "iceoryx_binding_c/server.h"
#include "iceoryx_binding_c/service_discovery.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/user_trigger.h"

/// @brief notification info handle
typedef const CLASS NotificationInfo* iox_notification_info_t;

/// @brief returns the id of the notification
/// @param[in] self handle to notification info
/// @return notificationId
uint64_t iox_notification_info_get_notification_id(iox_notification_info_t const self);

/// @brief does the notification originate from a certain subscriber
/// @param[in] self handle to notification info
/// @param[in] subscriber handle to the subscriber in question
/// @return true if the notification originates from the subscriber, otherwise false
bool iox_notification_info_does_originate_from_subscriber(iox_notification_info_t const self,
                                                          iox_sub_t const subscriber);

/// @brief does the notification originate from a certain user trigger
/// @param[in] self handle to notification info
/// @param[in] user_trigger handle to the user trigger in question
/// @return true if the notification originates from the user trigger, otherwise false
bool iox_notification_info_does_originate_from_user_trigger(iox_notification_info_t const self,
                                                            iox_user_trigger_t const user_trigger);

/// @brief does the notification originate from a certain client
/// @param[in] self handle to notification info
/// @param[in] client handle to the client in question
/// @return true if the notification originates from the client, otherwise false
bool iox_notification_info_does_originate_from_client(iox_notification_info_t const self, iox_client_t const client);

/// @brief does the notification originate from a certain server
/// @param[in] self handle to notification info
/// @param[in] server handle to the server in question
/// @return true if the notification originates from the server, otherwise false
bool iox_notification_info_does_originate_from_server(iox_notification_info_t const self, iox_server_t const server);

/// @brief does the notification originate from a certain service discovery
/// @param[in] self handle to notification info
/// @param[in] serviceDiscovery handle to serviceDiscovery in question
/// @return true if the notifiaction originates from the service discovery, otherwise false
bool iox_notification_info_does_originate_from_service_discovery(iox_notification_info_t const self,
                                                                 iox_service_discovery_t const serviceDiscovery);

/// @brief acquires the handle of the subscriber origin
/// @param[in] self handle to notification info
/// @return the handle to the subscriber if the notification originated from a subscriber, otherwise NULL
iox_sub_t iox_notification_info_get_subscriber_origin(iox_notification_info_t const self);

/// @brief acquires the handle of the user trigger origin
/// @param[in] self handle to notification info
/// @return the handle to the user trigger if the notification originated from a user trigger, otherwise NULL
iox_user_trigger_t iox_notification_info_get_user_trigger_origin(iox_notification_info_t const self);

/// @brief acquires the handle of the client origin
/// @param[in] self handle to notification info
/// @return the handle to the client if the notification originated from a client, otherwise NULL
iox_client_t iox_notification_info_get_client_origin(iox_notification_info_t const self);

/// @brief acquires the handle of the server origin
/// @param[in] self handle to notification info
/// @return the handle to the server if the notification originated from a server, otherwise NULL
iox_server_t iox_notification_info_get_server_origin(iox_notification_info_t const self);

/// @brief acquires the handle of the service discovery origin
/// @param[in] self handle to the notification info
/// @return the handle to the service discovery if the notification originated from a service discovery, otherwise NULL
iox_service_discovery_t iox_notification_info_get_service_discovery_origin(iox_notification_info_t const self);

/// @brief calls the callback of the notification
/// @param[in] self handle to notification info
void iox_notification_info_call(iox_notification_info_t const self);

#endif
