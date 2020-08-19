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
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/sleep_for.h"
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

void sending()
{
    PoshRuntime_getInstance("/iox-c-publisher");

    uint64_t historyRequest = 0u;
    struct PublisherPortData* publisher = Publisher_new("Radar", "FrontLeft", "Counter", historyRequest);

    Publisher_offer(publisher);

    uint32_t ct = 0u;

    while (!killswitch)
    {
        void* chunk = NULL;
        if (AllocationError_SUCCESS == Publisher_allocateChunk(publisher, &chunk, sizeof(struct CounterTopic)))
        {
            struct CounterTopic* sample = (struct CounterTopic*)chunk;

            sample->counter = ct;

            printf("Sending: %u\n", ct);

            Publisher_sendChunk(publisher, chunk);

            ++ct;

            sleepFor(1000);
        }
        else
        {
            printf("Failed to allocate chunk!");
        }
    }

    Publisher_stopOffer(publisher);
    Publisher_delete(publisher);
}

int main()
{
    signal(SIGINT, sigHandler);

    sending();

    return 0;
}
