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

#include "iceoryx_binding_c/posh_runtime.h"
#include "iceoryx_binding_c/subscriber.h"
#include "sleep_for.h"
#include "topic_data.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

bool killswitch = false;

static void sigHandler(int signalValue)
{
    (void)signalValue;
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

void receiving()
{
    iox_runtime_register("/iox-c-subscriber");

    uint64_t historyRequest = 0U;
    struct SubscriberPortData* subscriber = iox_sub_create("Radar", "FrontLeft", "Counter", historyRequest);
    iox_sub_subscribe(subscriber, 10);

    while (!killswitch)
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

        sleep_for(1000);
    }

    iox_sub_unsubscribe(subscriber);
    iox_sub_destroy(subscriber);
}

int main()
{
    signal(SIGINT, sigHandler);

    receiving();

    return 0;
}
