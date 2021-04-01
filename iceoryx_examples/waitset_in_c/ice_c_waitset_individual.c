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
#include "iceoryx_binding_c/event_info.h"
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

#define NUMBER_OF_EVENTS 3
#define NUMBER_OF_SUBSCRIBERS 2

iox_user_trigger_storage_t shutdownTriggerStorage;
iox_user_trigger_t shutdownTrigger;

static void sigHandler(int signalValue)
{
    // Ignore unused variable warning
    (void)signalValue;

    iox_user_trigger_trigger(shutdownTrigger);
}

int main()
{
    iox_runtime_init("iox-c-ex-waitset-individual");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

    // attach shutdownTrigger with no callback to handle CTRL+C
    iox_ws_attach_user_trigger_event(waitSet, shutdownTrigger, 0U, NULL);

    //// register signal after shutdownTrigger since we are using it in the handler
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // array where the subscriber are stored
    iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
    iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

    // create two subscribers, subscribe to the service and attach them to the waitset
    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.historyRequest = 1U;
    options.queueCapacity = 256U;
    options.nodeName = "iox-c-ex-waitset-individual-node1";

    subscriber[0] = iox_sub_init(&(subscriberStorage[0]), "Radar", "FrontLeft", "Counter", &options);

    options.nodeName = "iox-c-ex-waitset-individual-node2";
    subscriber[1] = iox_sub_init(&(subscriberStorage[1]), "Radar", "FrontLeft", "Counter", &options);

    iox_ws_attach_subscriber_state(waitSet, subscriber[0U], SubscriberState_HAS_DATA, 0U, NULL);
    iox_ws_attach_subscriber_state(waitSet, subscriber[1U], SubscriberState_HAS_DATA, 0U, NULL);


    uint64_t missedElements = 0U;
    uint64_t numberOfEvents = 0U;

    // array where all event infos from iox_ws_wait will be stored
    iox_event_info_t eventArray[NUMBER_OF_EVENTS];

    // event loop
    bool keepRunning = true;
    while (keepRunning)
    {
        numberOfEvents = iox_ws_wait(waitSet, eventArray, NUMBER_OF_EVENTS, &missedElements);

        for (uint64_t i = 0U; i < numberOfEvents; ++i)
        {
            iox_event_info_t event = eventArray[i];

            if (iox_event_info_does_originate_from_user_trigger(event, shutdownTrigger))
            {
                // CTRL+c was pressed -> exit
                keepRunning = false;
            }
            // process sample received by subscriber1
            else if (iox_event_info_does_originate_from_subscriber(event, subscriber[0U]))
            {
                const void* chunk;
                if (iox_sub_take_chunk(subscriber[0U], &chunk))
                {
                    printf("subscriber 1 received: %u\n", ((struct CounterTopic*)chunk)->counter);
                    fflush(stdout);

                    iox_sub_release_chunk(subscriber[0U], chunk);
                }
            }
            // dismiss sample received by subscriber2
            else if (iox_event_info_does_originate_from_subscriber(event, subscriber[1]))
            {
                // We need to release the samples to reset the event hasSamples
                // otherwise the WaitSet would notify us in `iox_ws_wait()` again
                // instantly.
                iox_sub_release_queued_chunks(subscriber[1U]);
                printf("subscriber 2 received something - dont care\n");
                fflush(stdout);
            }
        }
    }

    // cleanup all resources
    for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        iox_sub_deinit((iox_sub_t) & (subscriberStorage[i]));
    }

    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownTrigger);


    return 0;
}
