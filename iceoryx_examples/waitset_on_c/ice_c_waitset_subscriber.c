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

#define NUMBER_OF_CONDITIONS 2

bool killswitch = false;

iox_user_trigger_storage_t shutdowGuardStorage;
iox_user_trigger_t shutdownGuard;

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet;

iox_sub_t subscriber;

static void sigHandler(int signalValue)
{
    (void)signalValue;

    iox_user_trigger_trigger(shutdownGuard);
}

void receiving()
{
}

int main()
{
    iox_runtime_register("/iox-c-ex-waitset-gateway");

    waitSet = iox_ws_init(&waitSetStorage);
    shutdownGuard = iox_user_trigger_init(&shutdowGuardStorage);

    iox_user_trigger_attach_to_ws(shutdownGuard, waitSet, 0, NULL);

    //// register signal after guard condition since we are using it in the handler
    signal(SIGINT, sigHandler);

    uint64_t historyRequest = 1U;
    iox_sub_storage_t subscriberStorage;

    subscriber = iox_sub_init(&subscriberStorage, "Radar", "FrontLeft", "Counter", historyRequest);
    iox_sub_subscribe(subscriber, 1);

    iox_trigger_state_storage_t triggerArray[NUMBER_OF_CONDITIONS];
    uint64_t missedElements = 0U;
    uint64_t numberOfTriggeredConditions = 0U;
    do
    {
        // wait until an event has occurred
        numberOfTriggeredConditions =
            iox_ws_wait(waitSet, (iox_trigger_state_t)triggerArray, NUMBER_OF_CONDITIONS, &missedElements);
    } while (true);

    iox_sub_unsubscribe(subscriber);


    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownGuard);
    iox_sub_deinit(subscriber);


    return 0;
}
