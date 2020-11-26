// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#define NUMBER_OF_TRIGGER 2

iox_user_trigger_storage_t shutdowGuardStorage;
iox_user_trigger_t shutdownGuard;

iox_sub_t subscriber;

static void sigHandler(int signalValue)
{
    (void)signalValue;

    iox_user_trigger_trigger(shutdownGuard);
}

// The callback of the trigger. Every callback must have an argument which is
// a pointer to the origin of the Trigger. In our case the trigger origin is
// an iox_sub_t.
void subscriberCallback(iox_sub_t const subscriber)
{
    const void* chunk;
    if (iox_sub_get_chunk(subscriber, &chunk))
    {
        printf("subscriber: %p received %u\n", subscriber, ((struct CounterTopic*)chunk)->counter);

        iox_sub_release_chunk(subscriber, chunk);
    }
}

int main()
{
    iox_runtime_register("/iox-c-ex-waitset-gateway");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    shutdownGuard = iox_user_trigger_init(&shutdowGuardStorage);

    // attach shutdownGuard with no callback to handle CTRL+C
    iox_user_trigger_attach_to_ws(shutdownGuard, waitSet, 0, NULL);

    //// register signal after guard condition since we are using it in the handler
    signal(SIGINT, sigHandler);

    // array where the subscriber are stored
    iox_sub_storage_t subscriberStorage[2];

    // create subscriber and subscribe them to our service
    uint64_t historyRequest = 1U;
    for (uint64_t i = 0U; i < 2U; ++i)
    {
        subscriber = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", historyRequest);

        iox_sub_subscribe(subscriber, 256);
        iox_sub_attach_to_ws(subscriber, waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 1, subscriberCallback);
    }


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
            if (iox_trigger_state_does_originate_from_user_trigger((iox_trigger_state_t) & (triggerArray[i]),
                                                                   shutdownGuard))
            {
                // CTRL+c was pressed -> exit
                return 0;
            }
            else
            {
                // call the callback which was assigned to the trigger
                iox_trigger_state_call((iox_trigger_state_t) & (triggerArray[i]));
            }
        }
    }

    // cleanup all resources
    for (uint64_t i = 0U; i < 2U; ++i)
    {
        iox_sub_unsubscribe((iox_sub_t) & (subscriberStorage[i]));
        iox_sub_deinit((iox_sub_t) & (subscriberStorage[i]));
    }

    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownGuard);


    return 0;
}
