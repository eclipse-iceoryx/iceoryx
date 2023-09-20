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

int main(void)
{
    //! [initialization and shutdown handling]
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init("iox-c-waitset-individual");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    waitSetSigHandlerAccess = waitSet;
    //! [initialization and shutdown handling]

    //! [create and attach subscriber]
    // array where the subscriber are stored
    iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
    iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

    // create two subscribers, subscribe to the service and attach them to the waitset
    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.historyRequest = 1U;
    options.queueCapacity = 256U;
    options.nodeName = "iox-c-waitset-individual-node1";

    subscriber[0] = iox_sub_init(&(subscriberStorage[0]), "Radar", "FrontLeft", "Counter", &options);

    options.nodeName = "iox-c-waitset-individual-node2";
    subscriber[1] = iox_sub_init(&(subscriberStorage[1]), "Radar", "FrontLeft", "Counter", &options);

    iox_ws_attach_subscriber_state(waitSet, subscriber[0U], SubscriberState_HAS_DATA, 0U, NULL);
    iox_ws_attach_subscriber_state(waitSet, subscriber[1U], SubscriberState_HAS_DATA, 0U, NULL);
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

            if (iox_notification_info_does_originate_from_subscriber(notification, subscriber[0U]))
            {
                const void* userPayload;
                if (iox_sub_take_chunk(subscriber[0U], &userPayload))
                {
                    printf("subscriber 1 received: %u\n", ((struct CounterTopic*)userPayload)->counter);
                    fflush(stdout);

                    iox_sub_release_chunk(subscriber[0U], userPayload);
                }
            }
            // dismiss sample received by subscriber2
            else if (iox_notification_info_does_originate_from_subscriber(notification, subscriber[1]))
            {
                // We need to release the samples to reset the event hasSamples
                // otherwise the WaitSet would notify us in 'iox_ws_wait()' again
                // instantly.
                iox_sub_release_queued_chunks(subscriber[1U]);
                printf("subscriber 2 received something - dont care\n");
                fflush(stdout);
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
