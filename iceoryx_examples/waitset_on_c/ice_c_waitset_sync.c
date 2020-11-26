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
#include "iceoryx_binding_c/trigger_state.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
#include "sleep_for.h"
#include "topic_data.h"

#if !defined(_WIN32)
#include <pthread.h>
#endif
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_TRIGGER 2

iox_user_trigger_storage_t shutdowGuardStorage;
iox_user_trigger_t shutdownGuard;

iox_user_trigger_storage_t cyclicTriggerStorage;
iox_user_trigger_t cyclicTrigger;

bool keepRunning = true;

static void sigHandler(int signalValue)
{
    (void)signalValue;

    iox_user_trigger_trigger(shutdownGuard);
}

void cyclicRun(iox_user_trigger_t trigger)
{
    printf("activation callback\n");
    // after every call we have to reset the trigger otherwise the waitset
    // would immediately call us again since we still signal to the waitset that
    // we have been triggered (waitset is state based)
    iox_user_trigger_reset_trigger(trigger);
}

void* cyclicTriggerCallback(void* dontCare)
{
    (void)dontCare;
    while (keepRunning)
    {
        iox_user_trigger_trigger(cyclicTrigger);
        sleep_for(1000);
    }
    return NULL;
}

int main()
{
#if defined(_WIN32)
    printf("This example does not work on Windows. But you can easily adapt it for now by starting a windows thread "
           "which triggers the cyclicTrigger every second.\n");
#endif

    iox_runtime_register("/iox-c-ex-waitset-sync");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    shutdownGuard = iox_user_trigger_init(&shutdowGuardStorage);

    // attach shutdownGuard with no callback to handle CTRL+C
    iox_user_trigger_attach_to_ws(shutdownGuard, waitSet, 0, NULL);

    //// register signal after guard condition since we are using it in the handler
    signal(SIGINT, sigHandler);


    // create and attach the cyclicTrigger with a callback to
    // myCyclicRun
    cyclicTrigger = iox_user_trigger_init(&cyclicTriggerStorage);
    iox_user_trigger_attach_to_ws(cyclicTrigger, waitSet, 0, cyclicRun);

    // start a thread which triggers cyclicTrigger every second
#if !defined(_WIN32)
    pthread_t cyclicTriggerThread;
    if (pthread_create(&cyclicTriggerThread, NULL, cyclicTriggerCallback, NULL))
    {
        printf("failed to create thread\n");
        return -1;
    }
#endif

    uint64_t missedElements = 0U;
    uint64_t numberOfTriggeredConditions = 0U;

    // array where all trigger from iox_ws_wait will be stored
    iox_trigger_state_storage_t triggerArray[NUMBER_OF_TRIGGER];

    // event loop
    while (keepRunning)
    {
        numberOfTriggeredConditions =
            iox_ws_wait(waitSet, (iox_trigger_state_t)triggerArray, NUMBER_OF_TRIGGER, &missedElements);

        for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
        {
            iox_trigger_state_t trigger = (iox_trigger_state_t) & (triggerArray[i]);

            if (iox_trigger_state_does_originate_from_user_trigger(trigger, shutdownGuard))
            {
                // CTRL+c was pressed -> exit
                keepRunning = false;
            }
            else
            {
                // call myCyclicRun
                iox_trigger_state_call(trigger);
            }
        }
    }

    // cleanup all resources
#if !defined(_WIN32)
    pthread_join(cyclicTriggerThread, NULL);
#endif
    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownGuard);


    return 0;
}
