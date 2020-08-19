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
#include "iceoryx_binding_c/sleep_for.h"
#include "iceoryx_binding_c/subscriber.h"
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
    PoshRuntime_getInstance("/iox-c-publisher");

    uint64_t historyRequest = 0u;
    struct SubscriberPortData* subscriber = Subscriber_new("Radar", "FrontLeft", "Counter", historyRequest);
    Subscriber_subscribe(subscriber, 10);

    while (!killswitch)
    {
        if (SubscribeState_SUBSCRIBED == Subscriber_getSubscriptionState(subscriber))
        {
            const void* chunk = NULL;
            while (ChunkReceiveError_SUCCESS == Subscriber_getChunk(subscriber, &chunk))
            {
                const struct CounterTopic* sample = (const struct CounterTopic*)(chunk);
                printf("Receiving: %u\n", sample->counter);
                Subscriber_releaseChunk(subscriber, chunk);
            }
        }
        else
        {
            printf("Not subscribed!\n");
        }

        sleepFor(1000);
    }

    Subscriber_unsubscribe(subscriber);
    Subscriber_delete(subscriber);
}

int main()
{
    signal(SIGINT, sigHandler);

    receiving();

    return 0;
}
