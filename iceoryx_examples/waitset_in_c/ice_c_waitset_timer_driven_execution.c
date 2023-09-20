// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
#include <windows.h>
typedef HANDLE pthread_t;
#else
#include <pthread.h>
#endif
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#define NUMBER_OF_NOTIFICATIONS 2

volatile bool keepRunning = true;

iox_user_trigger_storage_t cyclicTriggerStorage;
iox_user_trigger_t cyclicTrigger;

volatile iox_ws_t waitSetSigHandlerAccess = NULL;

void sigHandler(int signalValue)
{
    // Ignore unused variable warning
    (void)signalValue;
    keepRunning = false;
    if (waitSetSigHandlerAccess)
    {
        iox_ws_mark_for_destruction(waitSetSigHandlerAccess);
    }
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
    int countdownToTrigger = 100;
    while (keepRunning)
    {
        if (countdownToTrigger == 0)
        {
            iox_user_trigger_trigger(cyclicTrigger);
            countdownToTrigger = 100;
        }
        sleep_for(10);
        --countdownToTrigger;
    }
    return NULL;
}

bool createThread(pthread_t* threadHandle, void* (*callback)(void*))
{
#if defined(_WIN32)
    threadHandle = CreateThread(NULL, 8192, callback, NULL, 0, NULL);
    return threadHandle != NULL;
#else
    return pthread_create(threadHandle, NULL, callback, NULL) == 0;
#endif
}

void joinThread(pthread_t threadHandle)
{
#if defined(_WIN32)
    WaitForMultipleObjects(1, &threadHandle, TRUE, INFINITE);
    CloseHandle(threadHandle);
#else
    pthread_join(threadHandle, NULL);
#endif
}

int main(void)
{
    //! [initialization and shutdown handling]
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    iox_runtime_init("iox-c-waitset-timer-driven-execution");

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
    waitSetSigHandlerAccess = waitSet;
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
    if (!createThread(&cyclicTriggerThread, cyclicTriggerCallback))
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

            if (iox_notification_info_does_originate_from_user_trigger(notification, cyclicTrigger))
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

    waitSetSigHandlerAccess = NULL; // invalidate for signal handler
    iox_ws_deinit(waitSet);

    iox_user_trigger_deinit(cyclicTrigger);
    //! [cleanup all resources]

    return 0;
}
