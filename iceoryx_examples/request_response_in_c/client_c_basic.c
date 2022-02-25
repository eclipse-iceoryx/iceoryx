// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_binding_c/client.h"
#include "iceoryx_binding_c/request_header.h"
#include "iceoryx_binding_c/response_header.h"
#include "iceoryx_binding_c/runtime.h"
#include "request_and_response_c_types.h"
#include "sleep_for.h"

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool keepRunning = true;
const char APP_NAME[] = "iox-c-request-response-client-basic";

void sigHandler(int signalValue)
{
    (void)signalValue;
    keepRunning = false;
}

int main()
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init(APP_NAME);

    iox_client_storage_t clientStorage;
    iox_client_t client = iox_client_init(&clientStorage, "Example", "Request-Response", "Add", NULL);

    uint64_t fibonacciLast = 0U;
    uint64_t fibonacciCurrent = 1U;
    int64_t requestSequenceId = 0;
    int64_t expectedResponseSequenceId = requestSequenceId;

    while (keepRunning)
    {
        struct AddRequest* request = NULL;
        enum iox_AllocationResult loanResult =
            iox_client_loan_request(client, (void**)&request, sizeof(struct AddRequest));

        if (loanResult == AllocationResult_SUCCESS)
        {
            iox_request_header_t requestHeader = iox_request_header_from_payload(request);
            iox_request_header_set_sequence_id(requestHeader, requestSequenceId);
            expectedResponseSequenceId = requestSequenceId;
            requestSequenceId += 1;
            request->augend = fibonacciLast;
            request->addend = fibonacciCurrent;
            printf("%s Send Request: %lu + %lu\n",
                   APP_NAME,
                   (unsigned long)fibonacciLast,
                   (unsigned long)fibonacciCurrent);
            iox_client_send(client, request);

            const uint32_t DELAY_TIME_IN_MS = 150U;
            sleep_for(DELAY_TIME_IN_MS);

            const struct AddResponse* response = NULL;
            while (iox_client_take_response(client, (const void**)&response) == ChunkReceiveResult_SUCCESS)
            {
                iox_const_response_header_t responseHeader = iox_response_header_from_payload_const(response);
                int64_t receivedSequenceId = iox_response_header_get_sequence_id_const(responseHeader);
                if (receivedSequenceId == expectedResponseSequenceId)
                {
                    fibonacciLast = fibonacciCurrent;
                    fibonacciCurrent = response->sum;
                    printf("%s Got Response: %lu\n", APP_NAME, (unsigned long)fibonacciCurrent);
                }
                else
                {
                    printf("Got Response with outdated sequence ID! Expected = %lu; Actual = %lu! -> skip\n",
                           (unsigned long)expectedResponseSequenceId,
                           (unsigned long)receivedSequenceId);
                }

                iox_client_release_response(client, response);
            }
        }
        else
        {
            printf("Could not allocate Request! Return value = %d\n", loanResult);
        }

        const uint32_t SLEEP_TIME_IN_MS = 950U;
        sleep_for(SLEEP_TIME_IN_MS);
    }

    iox_client_deinit(client);
}
