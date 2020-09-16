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

iox_wait_set_storage_t waitSetStorage;
iox_wait_set_t waitSet;

static void sigHandler(int signalValue)
{
    (void)signalValue;

    iox_guard_cond_trigger(guardCondition);
}

void receiving()
{
    iox_runtime_register("/iox-c-subscriber");

    waitSet = iox_wait_set_init(&waitSetStorage);
    guardCondition = iox_guard_cond_init(&guardConditionStorage);

    signal(SIGINT, sigHandler);

    uint64_t historyRequest = 0U;
    iox_sub_storage_t subscriberStorage;

    iox_sub_t subscriber = iox_sub_init(&subscriberStorage, "Radar", "FrontLeft", "Counter", historyRequest);
    iox_sub_subscribe(subscriber, 10);

    iox_wait_set_attach_condition(waitSet, (iox_cond_t)guardCondition);
    iox_wait_set_attach_condition(waitSet, (iox_cond_t)subscriber);

    iox_cond_t conditionArray[NUMBER_OF_CONDITIONS];
    uint64_t missedElements = 0u;
    while (true)
    {
        uint64_t numberOfTriggeredConditions =
            iox_wait_set_wait(waitSet, conditionArray, NUMBER_OF_CONDITIONS, &missedElements);

        for (uint64_t i = 0; i < numberOfTriggeredConditions; ++i)
        {
            if (conditionArray[i] == (iox_cond_t)guardCondition)
            {
                printf("Received exit signal!\n");
                break;
            }
            else if (conditionArray[i] == (iox_cond_t)subscriber)
            {
                if (SubscribeState_SUBSCRIBED == iox_sub_get_subscription_state(subscriber))
                {
                    const void* chunk = NULL;
                    while (ChunkReceiveResult_SUCCESS == iox_sub_get_chunk(subscriber, &chunk))
                    {
                        const struct CounterTopic* sample = (const struct CounterTopic*)(chunk);
                        printf("Receiving: %u\n", sample->counter);
                        iox_sub_release_chunk(subscriber, chunk);
                    }
                }
                else
                {
                    printf("Not subscribed!\n");
                }
            }
        }
    }

    iox_wait_set_detach_all_conditions(waitSet);
    iox_wait_set_deinit(waitSet);

    iox_guard_cond_deinit(guardCondition);

    iox_sub_unsubscribe(subscriber);
    iox_sub_deinit(subscriber);
}

int main()
{
    receiving();

    return 0;
}
