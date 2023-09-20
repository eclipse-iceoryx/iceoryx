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

#define NUMBER_OF_NOTIFICATIONS 5
#define NUMBER_OF_SUBSCRIBERS 4

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

int main(void)
{
    //! [initialization and shutdown handling]
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init("iox-c-waitset-grouping");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    waitSetSigHandlerAccess = waitSet;
    //! [initialization and shutdown handling]

    //! [create subscriber]
    // array where the subscribers are stored
    iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
    iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

    // create subscriber and subscribe them to our service
    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.historyRequest = 1U;
    options.queueCapacity = 256U;
    options.nodeName = "iox-c-waitset-grouping-node";
    for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        subscriber[i] = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", &options);
    }
    //! [create subscriber]

    //! [attach subscriber]
    const uint64_t FIRST_GROUP_ID = 123U;
    const uint64_t SECOND_GROUP_ID = 456U;

    // attach the first two subscribers to the waitset with a triggerid of FIRST_GROUP_ID
    for (uint64_t i = 0U; i < 2U; ++i)
    {
        iox_ws_attach_subscriber_state(waitSet, subscriber[i], SubscriberState_HAS_DATA, FIRST_GROUP_ID, NULL);
    }

    // attach the remaining subscribers to the waitset with a triggerid of SECOND_GROUP_ID
    for (uint64_t i = 2U; i < 4U; ++i)
    {
        iox_ws_attach_subscriber_state(waitSet, subscriber[i], SubscriberState_HAS_DATA, SECOND_GROUP_ID, NULL);
    }
    //! [attach subscriber]

    uint64_t missedElements = 0U;
    uint64_t numberOfNotifications = 0U;

    // array where all notification infos from iox_ws_wait will be stored
    iox_notification_info_t notificationArray[NUMBER_OF_NOTIFICATIONS];

    //! [event loop]
    while (keepRunning)
    {
        numberOfNotifications = iox_ws_wait(waitSet, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedElements);

        //! [handle events]
        for (uint64_t i = 0U; i < numberOfNotifications; ++i)
        {
            iox_notification_info_t notification = notificationArray[i];

            // we print the received data for the first group
            if (iox_notification_info_get_notification_id(notification) == FIRST_GROUP_ID)
            {
                iox_sub_t subscriber = iox_notification_info_get_subscriber_origin(notification);
                const void* userPayload;
                if (iox_sub_take_chunk(subscriber, &userPayload))
                {
                    printf("received: %u\n", ((struct CounterTopic*)userPayload)->counter);
                    fflush(stdout);

                    iox_sub_release_chunk(subscriber, userPayload);
                }
            }
            // dismiss the received data for the second group
            else if (iox_notification_info_get_notification_id(notification) == SECOND_GROUP_ID)
            {
                printf("dismiss data\n");
                iox_sub_t subscriber = iox_notification_info_get_subscriber_origin(notification);
                // We need to release the samples to reset the event hasSamples
                // otherwise the WaitSet would notify us in 'iox_ws_wait()' again
                // instantly.
                iox_sub_release_queued_chunks(subscriber);
            }
        }
        //! [handle events]
    }
    //! [event loop]

    //! [cleanup all resources]
    for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        iox_sub_deinit(subscriber[i]);
    }

    waitSetSigHandlerAccess = NULL; // invalidate for signal handler
    iox_ws_deinit(waitSet);
    //! [cleanup all resources]

    return 0;
}
