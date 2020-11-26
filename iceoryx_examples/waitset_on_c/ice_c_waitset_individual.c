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
#include "iceoryx_binding_c/trigger_state.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
#include "sleep_for.h"
#include "topic_data.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_TRIGGER 3
#define NUMBER_OF_SUBSCRIBER 2

iox_user_trigger_storage_t shutdowGuardStorage;
iox_user_trigger_t shutdownGuard;

static void sigHandler(int signalValue)
{
    (void)signalValue;

    iox_user_trigger_trigger(shutdownGuard);
}

int main()
{
    iox_runtime_register("/iox-c-ex-waitset-grouping");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    shutdownGuard = iox_user_trigger_init(&shutdowGuardStorage);

    // attach shutdownGuard with no callback to handle CTRL+C
    iox_user_trigger_attach_to_ws(shutdownGuard, waitSet, 0, NULL);

    //// register signal after guard condition since we are using it in the handler
    signal(SIGINT, sigHandler);

    // array where the subscriber are stored
    iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBER];
    iox_sub_t subscriber[NUMBER_OF_SUBSCRIBER];

    // create two subscribers, subscribe to the service and attach them to the waitset
    uint64_t historyRequest = 1U;
    subscriber[0] = iox_sub_init(&(subscriberStorage[0]), "Radar", "FrontLeft", "Counter", historyRequest);
    subscriber[1] = iox_sub_init(&(subscriberStorage[1]), "Radar", "FrontLeft", "Counter", historyRequest);

    iox_sub_subscribe(subscriber[0], 256);
    iox_sub_subscribe(subscriber[1], 256);

    iox_sub_attach_to_ws(subscriber[0], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 0, NULL);
    iox_sub_attach_to_ws(subscriber[1], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 0, NULL);


    uint64_t missedElements = 0U;
    uint64_t numberOfTriggeredConditions = 0U;

    // array where all trigger from iox_ws_wait will be stored
    iox_trigger_state_storage_t triggerArray[NUMBER_OF_TRIGGER];

    // event loop
    while (true)
    {
        numberOfTriggeredConditions =
            iox_ws_wait(waitSet, (iox_trigger_state_t)triggerArray, NUMBER_OF_TRIGGER, &missedElements);

        for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
        {
            iox_trigger_state_t trigger = (iox_trigger_state_t) & (triggerArray[i]);

            if (iox_trigger_state_does_originate_from_user_trigger(trigger, shutdownGuard))
            {
                // CTRL+c was pressed -> exit
                return 0;
            }
            // process sample received by subscriber1
            else if (iox_trigger_state_does_originate_from_subscriber(trigger, subscriber[0]))
            {
                const void* chunk;
                if (iox_sub_get_chunk(subscriber[0], &chunk))
                {
                    printf("subscriber 1 received: %u\n", ((struct CounterTopic*)chunk)->counter);

                    iox_sub_release_chunk(subscriber[0], chunk);
                }
            }
            // dismiss sample received by subscriber2
            else if (iox_trigger_state_does_originate_from_subscriber(trigger, subscriber[1]))
            {
                iox_sub_release_queued_chunks(subscriber[1]);
                printf("subscriber 2 received something - dont care\n");
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
    iox_user_trigger_deinit(shutdownGuard);


    return 0;
}
