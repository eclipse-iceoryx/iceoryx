// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/notification_info.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
#include "sleep_for.h"
#include "topic_data.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_NOTIFICATIONS 3
#define NUMBER_OF_SUBSCRIBERS 2

volatile bool keepRunning = true;

volatile iox_ws_t waitSetSigHandlerAccess = NULL;

void sigHandler(int signalValue)
{
    // Ignore unused variable warning
    (void)signalValue;
    keepRunning = false;
    if (waitSetSigHandlerAccess)
    {
        iox_ws_mark_for_destruction(waitSetSigHandlerAccess);
    }
}

// The callback of the trigger. Every callback must have an argument which is
// a pointer to the origin of the Trigger. In our case the trigger origin is
// an iox_sub_t.
//! [subscriber callback]
void subscriberCallback(iox_sub_t const subscriber, void* const contextData)
{
    if (contextData == NULL)
    {
        fprintf(stderr, "aborting subscriberCallback since contextData is a null pointer\n");
        return;
    }

    uint64_t* sumOfAllSamples = (uint64_t*)contextData;
    const void* userPayload = NULL;
    while (iox_sub_take_chunk(subscriber, &userPayload) == ChunkReceiveResult_SUCCESS)
    {
        printf("subscriber: %p received %u\n", (void*)subscriber, ((struct CounterTopic*)userPayload)->counter);
        fflush(stdout);

        iox_sub_release_chunk(subscriber, userPayload);
        ++(*sumOfAllSamples);
    }
}
//! [subscriber callback]

int main(void)
{
    //! [initialization and shutdown handling]
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init("iox-c-waitset-gateway");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    waitSetSigHandlerAccess = waitSet;
    //! [initialization and shutdown handling]

    //! [create and attach subscriber]
    uint64_t sumOfAllSamples = 0U;

    // array where the subscriber are stored
    iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
    iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

    // create subscriber and subscribe them to our service
    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.historyRequest = 1U;
    options.queueCapacity = 256U;
    options.nodeName = "iox-c-waitSet-gateway-node";
    for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        subscriber[i] = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", &options);

        iox_ws_attach_subscriber_event_with_context_data(
            waitSet, subscriber[i], SubscriberEvent_DATA_RECEIVED, 1U, subscriberCallback, &sumOfAllSamples);
    }
    //! [create and attach subscriber]

    //! [event loop]
    uint64_t missedElements = 0U;
    uint64_t numberOfNotifications = 0U;

    // array where all notification infos from iox_ws_wait will be stored
    iox_notification_info_t notificationArray[NUMBER_OF_NOTIFICATIONS];

    while (keepRunning)
    {
        numberOfNotifications = iox_ws_wait(waitSet, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedElements);

        //! [handle events]
        for (uint64_t i = 0U; i < numberOfNotifications; ++i)
        {
            iox_notification_info_t notification = notificationArray[i];

            // call the callback which was assigned to the event
            iox_notification_info_call(notification);

            printf("sum of all samples: %lu\n", (unsigned long)sumOfAllSamples);
            fflush(stdout);
        }
        //! [handle events]
    }
    //! [event loop]

    //! [cleanup all resources]
    for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        // not mandatory since iox_sub_deinit will detach the subscriber automatically
        // only added to present the full API
        iox_ws_detach_subscriber_event(waitSet, subscriber[i], SubscriberEvent_DATA_RECEIVED);
        iox_sub_deinit(subscriber[i]);
    }

    waitSetSigHandlerAccess = NULL; // invalidate for signal handler
    iox_ws_deinit(waitSet);
    //! [cleanup all resources]

    return 0;
}
