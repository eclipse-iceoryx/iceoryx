// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/notification_info.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"
#include "iceoryx_binding_c/wait_set.h"
#include "sleep_for.h"
#include "topic_data.h"

#if defined(_WIN32)
typedef long unsigned int pthread_t;
#else
#include <pthread.h>
#endif
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_NOTIFICATIONS 2

iox_user_trigger_storage_t shutdownTriggerStorage;
iox_user_trigger_t shutdownTrigger;

iox_user_trigger_storage_t cyclicTriggerStorage;
iox_user_trigger_t cyclicTrigger;

bool keepRunning = true;

static void sigHandler(int signalValue)
{
    // Ignore unused variable warning
    (void)signalValue;

    iox_user_trigger_trigger(shutdownTrigger);
}

void cyclicRun(iox_user_trigger_t trigger)
{
    (void)trigger;
    printf("activation callback\n");
    fflush(stdout);
}

void* cyclicTriggerCallback(void* dontCare)
{
    // Ignore unused variable warning
    (void)dontCare;
    while (keepRunning)
    {
        iox_user_trigger_trigger(cyclicTrigger);
        sleep_for(1000);
    }
    return NULL;
}

bool createThread(pthread_t* threadHandle, void* (*callback)(void*))
{
#if defined(_WIN32)
    return -1;
#else
    return pthread_create(threadHandle, NULL, callback, NULL);
#endif
}

int joinThread(pthread_t threadHandle)
{
#if defined(_WIN32)
    return -1;
#else
    return pthread_join(threadHandle, NULL);
#endif
}

int main(void)
{
#if defined(_WIN32)
    printf("This example does not work on Windows. But you can easily adapt it for now by starting a windows thread "
           "which triggers the cyclicTrigger every second.\n");
    return -1;
#endif

    //! [initialization and shutdown handling]
    iox_runtime_init("iox-c-waitset-timer-driven-execution");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

    // attach shutdownTrigger with no callback to handle CTRL+C
    iox_ws_attach_user_trigger_event(waitSet, shutdownTrigger, 0, NULL);

    // register signal after shutdownTrigger since we are using it in the handler
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    //! [initialization and shutdown handling]

    // create and attach the cyclicTrigger with a callback to
    // myCyclicRun
    //! [cyclic trigger]
    cyclicTrigger = iox_user_trigger_init(&cyclicTriggerStorage);
    iox_ws_attach_user_trigger_event(waitSet, cyclicTrigger, 0, cyclicRun);
    //! [cyclic trigger]

    // start a thread which triggers cyclicTrigger every second
    //! [cyclic trigger thread]
    pthread_t cyclicTriggerThread;
    if (createThread(&cyclicTriggerThread, cyclicTriggerCallback))
    {
        printf("failed to create thread\n");
        return -1;
    }
    //! [cyclic trigger thread]

    //! [event loop]
    uint64_t missedElements = 0U;
    uint64_t numberOfNotifications = 0U;

    // array where all notifications from iox_ws_wait will be stored
    iox_notification_info_t notificationArray[NUMBER_OF_NOTIFICATIONS];

    while (keepRunning)
    {
        numberOfNotifications = iox_ws_wait(waitSet, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedElements);

        //! [handle events]
        for (uint64_t i = 0U; i < numberOfNotifications; ++i)
        {
            iox_notification_info_t notification = notificationArray[i];

            if (iox_notification_info_does_originate_from_user_trigger(notification, shutdownTrigger))
            {
                // CTRL+C was pressed -> exit
                keepRunning = false;
            }
            else
            {
                // call myCyclicRun
                iox_notification_info_call(notification);
            }
        }
        //! [handle events]
    }
    //! [event loop]

    //! [cleanup all resources]
    joinThread(cyclicTriggerThread);
    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownTrigger);
    //! [cleanup all resources]

    return 0;
}
