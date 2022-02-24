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
#include "iceoryx_binding_c/service_discovery.h"
#include "sleep_for.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool keepRunning = true;

const char APP_NAME[] = "iox-c-find-service";

static void sigHandler(int signalValue)
{
    // Ignore unused variable warning
    (void)signalValue;
    // caught SIGINT or SIGTERM, now exit gracefully
    keepRunning = false;
}

void printSearchResult(const iox_service_description_t service)
{
    printf(
        "- Service: %s, Instance: %s, Event: %s\n", service.serviceString, service.instanceString, service.eventString);
}

void searchFrontCameras(const iox_service_description_t service, void* count)
{
    if (strcmp(service.instanceString, "FrontLeft") == 0 || strcmp(service.instanceString, "FrontRight") == 0)
    {
        ++*(uint64_t*)count;
    }
}

int main()
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init(APP_NAME);

    iox_service_discovery_storage_t storage;
    iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&storage);

    const uint64_t searchResultCapacity = 10U;
    iox_service_description_t searchResult[searchResultCapacity];
    uint64_t missedServices = 0U;
    uint64_t numberFoundServices = 0U;


    while (keepRunning)
    {
        uint64_t numberFrontCameras = 0U;

        printf("\n=========================================\n");

        printf("\nSearched for {'Radar', 'FrontLeft', 'Image'}. Found the following services:\n");
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, "Radar", "FrontLeft", "Image", printSearchResult, MessagingPattern_PUB_SUB);

        printf("\nSearched for {'Radar', *, *}. Found the following services:\n");
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, "Radar", NULL, NULL, printSearchResult, MessagingPattern_PUB_SUB);

        printf("\nSearched for {*, 'FrontLeft', *}. Found the following services:\n");
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, NULL, "FrontLeft", NULL, printSearchResult, MessagingPattern_PUB_SUB);

        printf("\nSearched for {*, 'FrontRight', 'Image'}. Found the following services:\n");
        iox_service_discovery_find_service_apply_callable(
            serviceDiscovery, NULL, "FrontRight", "Image", printSearchResult, MessagingPattern_PUB_SUB);

        numberFoundServices = iox_service_discovery_find_service(serviceDiscovery,
                                                                 "Camera",
                                                                 NULL,
                                                                 NULL,
                                                                 searchResult,
                                                                 searchResultCapacity,
                                                                 &missedServices,
                                                                 MessagingPattern_PUB_SUB);
        printf("\nSearched for {'Camera', *, *}. Found the following services:\n");
        for (uint64_t i = 0; i < numberFoundServices; ++i)
        {
            printSearchResult(searchResult[i]);
        }

        iox_service_discovery_find_service_apply_callable_with_context_data(serviceDiscovery,
                                                                            "Camera",
                                                                            NULL,
                                                                            NULL,
                                                                            searchFrontCameras,
                                                                            (void*)&numberFrontCameras,
                                                                            MessagingPattern_PUB_SUB);
        printf("\nFound %lu front cameras\n", numberFrontCameras);

        sleep_for(1000);
    }

    iox_service_discovery_deinit(serviceDiscovery);

    return 0;
}
