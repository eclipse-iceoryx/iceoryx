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

#include "iceoryx_binding_c/runtime.h"
//! [include service discovery]
#include "iceoryx_binding_c/service_discovery.h"
//! [include service discovery]
#include "sleep_for.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SEARCH_RESULT_CAPACITY 10

volatile bool keepRunning = true;

const char APP_NAME[] = "iox-c-find-service";

static void sigHandler(int signalValue)
{
    // Ignore unused variable warning
    (void)signalValue;
    // caught SIGINT or SIGTERM, now exit gracefully
    keepRunning = false;
}

//! [print function to be applied to search results]
void printSearchResult(const iox_service_description_t service)
{
    printf(
        "- Service: %s, Instance: %s, Event: %s\n", service.serviceString, service.instanceString, service.eventString);
}
//! [print function to be applied to search results]

//! [search function for all front devices]
void searchFrontDevices(const iox_service_description_t service, void* count)
{
    if (strncmp(service.instanceString, "FrontLeft", IOX_CONFIG_SERVICE_STRING_SIZE) == 0
        || strncmp(service.instanceString, "FrontRight", IOX_CONFIG_SERVICE_STRING_SIZE) == 0)
    {
        ++*(uint32_t*)count;
    }
}
//! [search function for all front devices]

int main(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init(APP_NAME);

    //! [create service discovery handle]
    iox_service_discovery_storage_t storage;
    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&storage);
    //! [create service discovery handle]

    iox_service_description_t searchResult[SEARCH_RESULT_CAPACITY];
    uint64_t missedServices = 0U;
    uint64_t numberFoundServices = 0U;

    const uint32_t WAIT_TIME_IN_MS = 1000U;

    while (keepRunning)
    {
        //! [store number of front cameras]
        uint32_t numberFrontCameras = 0U;
        //! [store number of front cameras]

        printf("\n=========================================\n");

        printf("\nSearched for {'Radar', 'FrontLeft', 'Objects'}. Found the following services:\n");
        //! [find service and apply callable]
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, "Radar", "FrontLeft", "Objects", printSearchResult, MessagingPattern_PUB_SUB);
        //! [find service and apply callable]

        printf("\nSearched for {'Radar', *, *}. Found the following services:\n");
        //! [search for all Radar services]
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, "Radar", NULL, NULL, printSearchResult, MessagingPattern_PUB_SUB);
        //! [search for all Radar services]

        printf("\nSearched for {*, 'FrontLeft', *}. Found the following services:\n");
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, NULL, "FrontLeft", NULL, printSearchResult, MessagingPattern_PUB_SUB);

        printf("\nSearched for {*, 'FrontRight', 'Image'}. Found the following services:\n");
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, NULL, "FrontRight", "Image", printSearchResult, MessagingPattern_PUB_SUB);

        //! [search for all Camera services]
        numberFoundServices = iox_service_discovery_find_service(serviceDiscovery,
                                                                 "Camera",
                                                                 NULL,
                                                                 NULL,
                                                                 searchResult,
                                                                 SEARCH_RESULT_CAPACITY,
                                                                 &missedServices,
                                                                 MessagingPattern_PUB_SUB);
        //! [search for all Camera services]
        printf("\nSearched for {'Camera', *, *}. Found the following services:\n");
        for (uint64_t i = 0; i < numberFoundServices; ++i)
        {
            printSearchResult(searchResult[i]);
        }

        //! [search for all front camera services]
        iox_service_discovery_find_service_apply_callable_with_context_data(
            serviceDiscovery, "Camera", NULL, NULL, searchFrontDevices, &numberFrontCameras, MessagingPattern_PUB_SUB);
        //! [search for all front camera services]
        printf("\nFound %u front cameras\n", numberFrontCameras);

        sleep_for(WAIT_TIME_IN_MS);
    }

    iox_service_discovery_deinit(serviceDiscovery);

    return 0;
}
