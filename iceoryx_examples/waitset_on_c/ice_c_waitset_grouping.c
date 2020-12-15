// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/trigger_info.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
#include "sleep_for.h"
#include "topic_data.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_TRIGGER 5
#define NUMBER_OF_SUBSCRIBER 4

iox_user_trigger_storage_t shutdownTriggerStorage;
iox_user_trigger_t shutdownTrigger;

static void sigHandler(int signalValue)
{
    (void)signalValue;

    iox_user_trigger_trigger(shutdownTrigger);
}

int main()
{
    iox_runtime_init("/iox-c-ex-waitset-grouping");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

    // attach shutdownTrigger with no callback to handle CTRL+C
    iox_user_trigger_attach_to_waitset(shutdownTrigger, waitSet, 0, NULL);

    //// register signal after shutdownTrigger since we are using it in the handler
    signal(SIGINT, sigHandler);

    // array where the subscriber are stored
    iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBER];
    iox_sub_t subscriber[NUMBER_OF_SUBSCRIBER];

    // create subscriber and subscribe them to our service
    uint64_t historyRequest = 1U;
    for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
    {
        subscriber[i] = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", historyRequest);

        iox_sub_subscribe(subscriber[i], 256);
    }

    const uint64_t FIRST_GROUP_ID = 123;
    const uint64_t SECOND_GROUP_ID = 456;

    // attach the first two subscriber to waitset with a triggerid of FIRST_GROUP_ID
    for (uint64_t i = 0U; i < 2U; ++i)
    {
        iox_sub_attach_to_waitset(subscriber[i], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, FIRST_GROUP_ID, NULL);
    }

    // attach the remaining subscribers to waitset with a triggerid of SECOND_GROUP_ID
    for (uint64_t i = 2U; i < 4U; ++i)
    {
        iox_sub_attach_to_waitset(subscriber[i], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, SECOND_GROUP_ID, NULL);
    }


    uint64_t missedElements = 0U;
    uint64_t numberOfTriggeredConditions = 0U;

    // array where all trigger from iox_ws_wait will be stored
    iox_trigger_info_t triggerArray[NUMBER_OF_TRIGGER];

    // event loop
    bool keepRunning = true;
    while (keepRunning)
    {
        numberOfTriggeredConditions = iox_ws_wait(waitSet, triggerArray, NUMBER_OF_TRIGGER, &missedElements);

        for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
        {
            iox_trigger_info_t trigger = triggerArray[i];

            if (iox_trigger_info_does_originate_from_user_trigger(trigger, shutdownTrigger))
            {
                // CTRL+c was pressed -> exit
                keepRunning = false;
            }
            // we print the received data for the first group
            else if (iox_trigger_info_get_trigger_id(trigger) == FIRST_GROUP_ID)
            {
                iox_sub_t subscriber = iox_trigger_info_get_subscriber_origin(trigger);
                const void* chunk;
                if (iox_sub_get_chunk(subscriber, &chunk))
                {
                    printf("received: %u\n", ((struct CounterTopic*)chunk)->counter);

                    iox_sub_release_chunk(subscriber, chunk);
                }
            }
            // dismiss the received data for the second group
            else if (iox_trigger_info_get_trigger_id(trigger) == SECOND_GROUP_ID)
            {
                printf("dismiss data\n");
                iox_sub_t subscriber = iox_trigger_info_get_subscriber_origin(trigger);
                // We need to release the samples to reset the trigger hasNewSamples
                // otherwise the WaitSet would notify us in `iox_ws_wait()` again
                // instantly.
                iox_sub_release_queued_chunks(subscriber);
            }
        }
    }

    // cleanup all resources
    for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
    {
        iox_sub_unsubscribe((iox_sub_t) & (subscriberStorage[i]));
        iox_sub_deinit((iox_sub_t) & (subscriberStorage[i]));
    }

    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownTrigger);


    return 0;
}
