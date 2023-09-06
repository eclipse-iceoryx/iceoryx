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

#include "iceoryx_binding_c/request_header.h"
#include "iceoryx_binding_c/response_header.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/server.h"
#include "request_and_response_c_types.h"
#include "sleep_for.h"

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

volatile bool keepRunning = true;
const char APP_NAME[] = "iox-c-request-response-server-basic";

void sigHandler(int signalValue)
{
    (void)signalValue;
    keepRunning = false;
}

int main(void)
{
    //! [register signal handler and init runtime]
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init(APP_NAME);
    //! [register signal handler and init runtime]

    //! [init server]
    iox_server_storage_t serverStorage;
    iox_server_t server = iox_server_init(&serverStorage, "Example", "Request-Response", "Add", NULL);
    //! [init server]

    //! [main loop]
    while (keepRunning)
    {
        //! [process request]
        const struct AddRequest* request = NULL;
        if (iox_server_take_request(server, (const void**)&request) == ServerRequestResult_SUCCESS)
        {
            printf("%s Got Request: %lu + %lu\n",
                   APP_NAME,
                   (unsigned long)request->augend,
                   (unsigned long)request->addend);

            struct AddResponse* response = NULL;
            enum iox_AllocationResult loanResult =
                iox_server_loan_response(server, request, (void**)&response, sizeof(struct AddResponse));
            if (loanResult == AllocationResult_SUCCESS)
            {
                response->sum = request->augend + request->addend;
                printf("%s Send Response: %lu\n", APP_NAME, (unsigned long)response->sum);
                enum iox_ServerSendResult sendResult = iox_server_send(server, response);
                if (sendResult != ServerSendResult_SUCCESS)
                {
                    printf("Error sending Response! Error code: %d\n", sendResult);
                }
            }
            else
            {
                printf("%s Could not allocate Response! Error code: %d\n", APP_NAME, loanResult);
            }

            iox_server_release_request(server, request);
        }
        //! [process request]

        const uint32_t SLEEP_TIME_IN_MS = 100U;
        sleep_for(SLEEP_TIME_IN_MS);
    }
    //! [main loop]

    //! [cleanup]
    iox_server_deinit(server);
    //! [cleanup]
}
