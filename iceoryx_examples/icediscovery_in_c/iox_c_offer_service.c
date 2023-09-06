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

#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/runtime.h"
#include "sleep_for.h"

#include <signal.h>
#include <stdbool.h>

#define NUMBER_OF_CAMERA_PUBLISHERS 5

volatile bool keepRunning = true;

const char APP_NAME[] = "iox-c-offer-service";

static void sigHandler(int signalValue)
{
    // Ignore unused variable warning
    (void)signalValue;
    // caught SIGINT or SIGTERM, now exit gracefully
    keepRunning = false;
}

int main(void)
{
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init(APP_NAME);

    iox_pub_storage_t publisherStorage;
    iox_pub_options_t options;
    iox_pub_options_init(&options);
    iox_pub_t radarLeft = iox_pub_init(&publisherStorage, "Radar", "FrontLeft", "Objects", &options);
    iox_pub_t radarRight = iox_pub_init(&publisherStorage, "Radar", "FrontRight", "Objects", &options);
    iox_pub_t lidarLeft = iox_pub_init(&publisherStorage, "Lidar", "FrontLeft", "Counter", &options);

    iox_pub_storage_t cameraPublisherStorage[NUMBER_OF_CAMERA_PUBLISHERS];
    iox_pub_t cameraPublishers[NUMBER_OF_CAMERA_PUBLISHERS];
    cameraPublishers[0] = iox_pub_init(&cameraPublisherStorage[0], "Camera", "FrontLeft", "Counter", &options);
    cameraPublishers[1] = iox_pub_init(&cameraPublisherStorage[1], "Camera", "FrontLeft", "Image", &options);
    cameraPublishers[2] = iox_pub_init(&cameraPublisherStorage[2], "Camera", "FrontRight", "Counter", &options);
    cameraPublishers[3] = iox_pub_init(&cameraPublisherStorage[3], "Camera", "FrontRight", "Image", &options);
    cameraPublishers[4] = iox_pub_init(&cameraPublisherStorage[4], "Camera", "BackLeft", "Image", &options);

    const uint32_t WAIT_TIME_IN_MS = 1000U;
    bool offer = false;
    while (keepRunning)
    {
        if (offer)
        {
            for (int i = 0; i < NUMBER_OF_CAMERA_PUBLISHERS; ++i)
            {
                iox_pub_offer(cameraPublishers[i]);
            }
        }
        else
        {
            for (int i = 0; i < NUMBER_OF_CAMERA_PUBLISHERS; ++i)
            {
                iox_pub_stop_offer(cameraPublishers[i]);
            }
        }
        offer = !offer;
        sleep_for(WAIT_TIME_IN_MS);
    }

    iox_pub_deinit(radarLeft);
    iox_pub_deinit(radarRight);
    iox_pub_deinit(lidarLeft);
    for (int i = 0; i < NUMBER_OF_CAMERA_PUBLISHERS; ++i)
    {
        iox_pub_deinit(cameraPublishers[i]);
    }

    return 0;
}
