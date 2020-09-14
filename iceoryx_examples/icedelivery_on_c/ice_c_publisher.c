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

#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/runtime.h"
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

void sending()
{
    iox_runtime_register("/iox-c-publisher");

    uint64_t historyRequest = 0U;
    iox_pub_storage_t publisherStorage;
    iox_pub_t publisher = iox_pub_init(&publisherStorage, "Radar", "FrontLeft", "Counter", historyRequest);

    iox_pub_offer(publisher);

    uint32_t ct = 0U;

    while (!killswitch)
    {
        void* chunk = NULL;
        if (AllocationResult_SUCCESS == iox_pub_allocate_chunk(publisher, &chunk, sizeof(struct CounterTopic)))
        {
            struct CounterTopic* sample = (struct CounterTopic*)chunk;

            sample->counter = ct;

            printf("Sending: %u\n", ct);

            iox_pub_send_chunk(publisher, chunk);

            ++ct;

            sleep_for(1000);
        }
        else
        {
            printf("Failed to allocate chunk!");
        }
    }

    iox_pub_stop_offer(publisher);
    iox_pub_deinit(publisher);
}

int main()
{
    signal(SIGINT, sigHandler);

    sending();

    return 0;
}
