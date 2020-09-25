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

#include "iceoryx_binding_c/guard_condition.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/wait_set.h"
#include "sleep_for.h"
#include "topic_data.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_CONDITIONS 2

bool killswitch = false;

iox_guard_cond_storage_t guardConditionStorage;
iox_guard_cond_t guardCondition;

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet;

iox_sub_t subscriber;

static void sigHandler(int signalValue)
{
    (void)signalValue;

    iox_guard_cond_trigger(guardCondition);
}

bool callback(iox_cond_t* conditions, const uint64_t numberOfConditions)
{
    for (uint64_t i = 0; i < numberOfConditions; ++i)
    {
        // if the guard condition was triggered we return false, leave the loop
        // and cleanup all resources
        if (conditions[i] == (iox_cond_t)guardCondition)
        {
            printf("Received exit signal!\n");
            return false;
        }
        // if a subscriber was triggered we receive a sample and print it
        // to the terminal
        else if (conditions[i] == (iox_cond_t)subscriber)
        {
            if (SubscribeState_SUBSCRIBED == iox_sub_get_subscription_state(subscriber))
            {
                const void* chunk = NULL;
                while (ChunkReceiveResult_SUCCESS == iox_sub_get_chunk(subscriber, &chunk))
                {
                    const struct TopicData* sample = (const struct TopicData*)(chunk);
                    printf("Receiving: %s\n", sample->message);
                    iox_sub_release_chunk(subscriber, chunk);
                }
            }
            else
            {
                printf("Not subscribed!\n");
            }
        }
    }

    return true;
}

void receiving()
{
    iox_runtime_register("/iox-c-subscriber");

    // initialize wait set and guard condition
    waitSet = iox_ws_init(&waitSetStorage);
    guardCondition = iox_guard_cond_init(&guardConditionStorage);

    // register signal after guard condition since we are using it in the handler
    signal(SIGINT, sigHandler);

    uint64_t historyRequest = 0U;
    iox_sub_storage_t subscriberStorage;

    subscriber = iox_sub_init(&subscriberStorage, "Radar", "FrontLeft", "Counter", historyRequest);
    iox_sub_subscribe(subscriber, 10);

    // attach guard condition to our wait set, used to signal the wait set that
    // we would like to terminate the process
    iox_ws_attach_condition(waitSet, (iox_cond_t)guardCondition);

    // attach subscriber to our wait set. if the subscriber receives a sample
    // it will trigger the wait set
    iox_ws_attach_condition(waitSet, (iox_cond_t)subscriber);


    iox_cond_t conditionArray[NUMBER_OF_CONDITIONS];
    uint64_t missedElements = 0U;
    uint64_t numberOfTriggeredConditions = 0U;
    do
    {
        // wait until an event has occurred
        numberOfTriggeredConditions = iox_ws_wait(waitSet, conditionArray, NUMBER_OF_CONDITIONS, &missedElements);

        // call our callback, if the guard condition was triggered it returns false
    } while (callback(conditionArray, numberOfTriggeredConditions));

    iox_sub_unsubscribe(subscriber);

    // detach all conditions before we deinitialize and destroy them
    iox_ws_detach_all_conditions(waitSet);

    iox_ws_deinit(waitSet);
    iox_guard_cond_deinit(guardCondition);
    iox_sub_deinit(subscriber);
}

int main()
{
    receiving();

    return 0;
}
