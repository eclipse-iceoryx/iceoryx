# WaitSet in C

## Thread Safety

The _WaitSet_ is **not** thread-safe!

- It is **not** allowed to attach or detach _Triggerable_
   classes with `iox_ws_attach_**` or `iox_ws_detach_**` when another thread is currently
   waiting for notifications with `iox_ws_wait` or `iox_ws_timed_wait`.
- Do **not** call any of the `iox_ws_` functions concurrently.

The _TriggerHandle_ on the other hand, is thread-safe! Therefore you are allowed to
attach/detach a _TriggerHandle_ to a _Triggerable_ while another thread may
trigger the _TriggerHandle_.

## Introduction

A detailed introduction into the WaitSet nomenclature and topic can be found in the
[waitset C++ example](../waitset).
Here we will only introduce the C API and not the WaitSet in general. For this, we will
take a look at the same use case as the
[waitset C++ example](../waitset).
The examples are structured in the same way as the C++ ones.

## Expected Output

[![asciicast](https://asciinema.org/a/VX5S5jP6DAzAi4YID1GuJqfjW.svg)](https://asciinema.org/a/VX5S5jP6DAzAi4YID1GuJqfjW)

## Code Walkthrough

!!! warning
    Please be aware of the thread-safety restrictions of the _WaitSet_ and
    read the [Thread Safety](#thread-safety) chapter carefully.

To run an example you need a running `iox-roudi` and the waitset publisher
`iox-c-waitset-publisher`. They are identical to the ones introduced in the
[icedelivery C example](../icedelivery_in_c).

### Gateway

Let's say we would like to write a gateway and would like to forward every
incoming message from a subscriber with the same callback. For instance we could perform
a memcopy of the received data into a specific struct. Additionally, we would
like to count all processed samples. Therefore we provide an extra void pointer
argument called `contextData` which is a pointer to an `uint64_t`.

This could be performed by a function that we attach to an event as a
callback. In our case, we have the function `subscriberCallback` that
prints out the subscriber pointer and the content of the received sample.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_gateway.c][subscriber callback]-->
```c
void subscriberCallback(iox_sub_t const subscriber, void* const contextData)
{
    if (contextData == NULL)
    {
        fprintf(stderr, "aborting subscriberCallback since contextData is a null pointer\n");
        return;
    }

    uint64_t* sumOfAllSamples = (uint64_t*)contextData;
    const void* userPayload = NULL;
    while (iox_sub_take_chunk(subscriber, &userPayload) == ChunkReceiveResult_SUCCESS)
    {
        printf("subscriber: %p received %u\n", (void*)subscriber, ((struct CounterTopic*)userPayload)->counter);
        fflush(stdout);

        iox_sub_release_chunk(subscriber, userPayload);
        ++(*sumOfAllSamples);
    }
}
```

Since we attach the `SubscriberEvent_DATA_RECEIVED` event to the _WaitSet_ that
notifies us just once when data was received we have to gather and process all chunks.
One will never miss chunks since the event notification is reset after a call to
`iox_ws_wait` or `iox_ws_timed_wait` which we introduce below.

After we registered our runtime we set up some `waitSetStorage`, initialize the _WaitSet_
and let `waitSetSigHandlerAccess` point to `waitSet`. `waitSetSigHandlerAccess` is used by
the signal handler to initiate a graceful shutdown.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_gateway.c][initialization and shutdown handling]-->
```c
signal(SIGINT, sigHandler);
signal(SIGTERM, sigHandler);

iox_runtime_init("iox-c-waitset-gateway");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
waitSetSigHandlerAccess = waitSet;
```

In the next steps, we define `sumOfAllSamples`, create two subscribers with `iox_sub_init`,
subscribe them to our topic
and attach the event `SubscriberEvent_DATA_RECEIVED` to the _WaitSet_ with
the `subscriberCallback`, an event id `1U` and a pointer to our user defined
context data `sumOfAllSamples` which is then provided as argument for the callback.

!!! attention
    The user has to ensure that the contextData (`sumOfAllSamples`) in
    `iox_ws_attach_subscriber_event_with_context_data` lives as long as the
    attachment, with its callback, is attached otherwise the callback context
    data pointer is dangling.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_gateway.c][create and attach subscriber]-->
```c
uint64_t sumOfAllSamples = 0U;

// array where the subscriber are stored
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

// create subscriber and subscribe them to our service
iox_sub_options_t options;
iox_sub_options_init(&options);
options.historyRequest = 1U;
options.queueCapacity = 256U;
options.nodeName = "iox-c-waitSet-gateway-node";
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriber[i] = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", &options);

    iox_ws_attach_subscriber_event_with_context_data(
        waitSet, subscriber[i], SubscriberEvent_DATA_RECEIVED, 1U, subscriberCallback, &sumOfAllSamples);
}
```

Now that everything is set up we enter the event loop. It always starts with
a call to `iox_ws_wait`, a blocking call which returns us the number
of occurred notifications.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_gateway.c][[event loop]]-->
```c
uint64_t missedElements = 0U;
uint64_t numberOfNotifications = 0U;

// array where all notification infos from iox_ws_wait will be stored
iox_notification_info_t notificationArray[NUMBER_OF_NOTIFICATIONS];

while (keepRunning)
{
    numberOfNotifications = iox_ws_wait(waitSet, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedElements);
    // ...
}
```

The events which have occurred are stored in the `notificationArray`. We iterate through
it and call the callback with `iox_notification_info_call(notification)`.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_gateway.c][handle events]-->
```c
for (uint64_t i = 0U; i < numberOfNotifications; ++i)
{
    iox_notification_info_t notification = notificationArray[i];

    // call the callback which was assigned to the event
    iox_notification_info_call(notification);

    printf("sum of all samples: %lu\n", (unsigned long)sumOfAllSamples);
    fflush(stdout);
}
```

Before we can close the program, we cleanup all resources and set `waitSetSigHandlerAccess` to
`NULL` to prevent the signal handler to access an invalid waitset.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_gateway.c][cleanup all resources]-->
```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    // not mandatory since iox_sub_deinit will detach the subscriber automatically
    // only added to present the full API
    iox_ws_detach_subscriber_event(waitSet, subscriber[i], SubscriberEvent_DATA_RECEIVED);
    iox_sub_deinit(subscriber[i]);
}

waitSetSigHandlerAccess = NULL; // invalidate for signal handler
iox_ws_deinit(waitSet);
```

### Grouping

In this scenario, we have two groups of subscribers. We are interested in the
data of the first group and would like to print them onto the console and the
data of the second group should be discarded.

We start like in every example with registering the signal handler, initializing
the runtime and creating the _WaitSet_.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_grouping.c][initialization and shutdown handling]-->
```c
signal(SIGINT, sigHandler);
signal(SIGTERM, sigHandler);

iox_runtime_init("iox-c-waitset-grouping");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
waitSetSigHandlerAccess = waitSet;
```

After that we can create a list of subscribers and subscribe them to our topic.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_grouping.c][create subscriber]-->
```c
// array where the subscribers are stored
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

// create subscriber and subscribe them to our service
iox_sub_options_t options;
iox_sub_options_init(&options);
options.historyRequest = 1U;
options.queueCapacity = 256U;
options.nodeName = "iox-c-waitset-grouping-node";
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriber[i] = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", &options);
}
```

To distinct our two groups we set the eventId of the first group to
`123` and of the second group to `456`. The first two subscribers are attached with
the `SubscriberState_HAS_DATA` state and the event id of the first group to our waitset.
The third and forth subscriber are attached to the same
waitset under the second group id.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_grouping.c][attach subscriber]-->
```c
const uint64_t FIRST_GROUP_ID = 123U;
const uint64_t SECOND_GROUP_ID = 456U;

// attach the first two subscribers to the waitset with a triggerid of FIRST_GROUP_ID
for (uint64_t i = 0U; i < 2U; ++i)
{
    iox_ws_attach_subscriber_state(waitSet, subscriber[i], SubscriberState_HAS_DATA, FIRST_GROUP_ID, NULL);
}

// attach the remaining subscribers to the waitset with a triggerid of SECOND_GROUP_ID
for (uint64_t i = 2U; i < 4U; ++i)
{
    iox_ws_attach_subscriber_state(waitSet, subscriber[i], SubscriberState_HAS_DATA, SECOND_GROUP_ID, NULL);
}
```

We are again ready for our event loop. We start as usual by setting the array
of notifications by calling `iox_ws_wait`.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_grouping.c][[event loop]]-->
```c
while (keepRunning)
{
    numberOfNotifications = iox_ws_wait(waitSet, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedElements);
    // ...
}
```

We iterate through the array and check if an event is from the first group by calling
`iox_notification_info_get_event_id` and compare the result with `FIRST_GROUP_ID`.
If that is the case we acquire the subscriber handle with
`iox_notification_info_get_subscriber_origin`. This allows us to receive the new
sample and to print the result to the console.
The second group is handled in the same way. But we do not print the new samples
to screen, we just discard them.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_grouping.c][handle events]-->
```c
for (uint64_t i = 0U; i < numberOfNotifications; ++i)
{
    iox_notification_info_t notification = notificationArray[i];

    // we print the received data for the first group
    if (iox_notification_info_get_notification_id(notification) == FIRST_GROUP_ID)
    {
        iox_sub_t subscriber = iox_notification_info_get_subscriber_origin(notification);
        const void* userPayload;
        if (iox_sub_take_chunk(subscriber, &userPayload))
        {
            printf("received: %u\n", ((struct CounterTopic*)userPayload)->counter);
            fflush(stdout);

            iox_sub_release_chunk(subscriber, userPayload);
        }
    }
    // dismiss the received data for the second group
    else if (iox_notification_info_get_notification_id(notification) == SECOND_GROUP_ID)
    {
        printf("dismiss data\n");
        iox_sub_t subscriber = iox_notification_info_get_subscriber_origin(notification);
        // We need to release the samples to reset the event hasSamples
        // otherwise the WaitSet would notify us in 'iox_ws_wait()' again
        // instantly.
        iox_sub_release_queued_chunks(subscriber);
    }
}
```

In the case of the `SECOND_GROUP_ID` we have to release all queued chunks otherwise
the _WaitSet_ would notify us right away since the `SubscriberState_HAS_DATA` still
persists.

The last thing we have to do is to cleanup all the acquired resources.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_grouping.c][cleanup all resources]-->
```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    iox_sub_deinit(subscriber[i]);
}

waitSetSigHandlerAccess = NULL; // invalidate for signal handler
iox_ws_deinit(waitSet);
```

### Individual

We also can handle every event individually, for instance when you would like
to have a different reaction for every subscriber which has received a sample.
One way would be to assign every subscriber a different callback, here we look
at a different approach. We check if the event originated from a specific
subscriber and then perform the calls on that subscriber directly.

We start as usual with the setup of the signal handler and _WaitSet_.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_individual.c][initialization and shutdown handling]-->
```c
signal(SIGINT, sigHandler);
signal(SIGTERM, sigHandler);

iox_runtime_init("iox-c-waitset-individual");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
waitSetSigHandlerAccess = waitSet;
```

Now we create two subscribers, subscribe them to our topic and attach them to
the waitset without a callback and with the same trigger id.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_individual.c][create and attach subscriber]-->
```c
// array where the subscriber are stored
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

// create two subscribers, subscribe to the service and attach them to the waitset
iox_sub_options_t options;
iox_sub_options_init(&options);
options.historyRequest = 1U;
options.queueCapacity = 256U;
options.nodeName = "iox-c-waitset-individual-node1";

subscriber[0] = iox_sub_init(&(subscriberStorage[0]), "Radar", "FrontLeft", "Counter", &options);

options.nodeName = "iox-c-waitset-individual-node2";
subscriber[1] = iox_sub_init(&(subscriberStorage[1]), "Radar", "FrontLeft", "Counter", &options);

iox_ws_attach_subscriber_state(waitSet, subscriber[0U], SubscriberState_HAS_DATA, 0U, NULL);
iox_ws_attach_subscriber_state(waitSet, subscriber[1U], SubscriberState_HAS_DATA, 0U, NULL);
```

We are ready to start the event loop. We begin by acquiring the array of all
the triggered triggers.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_individual.c][[event loop]]-->
```c
uint64_t missedElements = 0U;
uint64_t numberOfNotifications = 0U;

// array where all notification infos from iox_ws_wait will be stored
iox_notification_info_t notificationArray[NUMBER_OF_NOTIFICATIONS];

while (keepRunning)
{
    numberOfNotifications = iox_ws_wait(waitSet, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedElements);
    // ...
}
```

We use `iox_notification_info_does_originate_from_subscriber`
to identify the event that originated from a specific subscriber. If it originated
from the first subscriber we print the received data to the console, if it
originated from the second subscriber we discard the data.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_individual.c][handle events]-->
```c
for (uint64_t i = 0U; i < numberOfNotifications; ++i)
{
    iox_notification_info_t notification = notificationArray[i];

    if (iox_notification_info_does_originate_from_subscriber(notification, subscriber[0U]))
    {
        const void* userPayload;
        if (iox_sub_take_chunk(subscriber[0U], &userPayload))
        {
            printf("subscriber 1 received: %u\n", ((struct CounterTopic*)userPayload)->counter);
            fflush(stdout);

            iox_sub_release_chunk(subscriber[0U], userPayload);
        }
    }
    // dismiss sample received by subscriber2
    else if (iox_notification_info_does_originate_from_subscriber(notification, subscriber[1]))
    {
        // We need to release the samples to reset the event hasSamples
        // otherwise the WaitSet would notify us in 'iox_ws_wait()' again
        // instantly.
        iox_sub_release_queued_chunks(subscriber[1U]);
        printf("subscriber 2 received something - dont care\n");
        fflush(stdout);
    }
}
```

We conclude the example as always, by cleaning up the resources.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_individual.c][cleanup all resources]-->
```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    iox_sub_deinit(subscriber[i]);
}

waitSetSigHandlerAccess = NULL; // invalidate for signal handler
iox_ws_deinit(waitSet);
```

### Timer Driven Execution

In this example, we demonstrate how you can use the _WaitSet_ to trigger a cyclic
call every second. We use a user trigger which will be triggered in a separate
thread every second to signal the _WaitSet_ that it's time for the next run.
Additionally, we attach a callback (`cyclicRun`) to this user trigger
so that the event can directly call the cyclic call.

We start as usual with the setup of the signal handler and _WaitSet_.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_timer_driven_execution.c][initialization and shutdown handling]-->
```c
signal(SIGINT, sigHandler);
signal(SIGTERM, sigHandler);

iox_runtime_init("iox-c-waitset-timer-driven-execution");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
waitSetSigHandlerAccess = waitSet;
```

Now we create our cyclic trigger and attach it to our waitset with an eventId
of `0` and the callback `cyclicRun`.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_timer_driven_execution.c][cyclic trigger]-->
```c
cyclicTrigger = iox_user_trigger_init(&cyclicTriggerStorage);
iox_ws_attach_user_trigger_event(waitSet, cyclicTrigger, 0, cyclicRun);
```

The thread which will trigger the `cyclicTrigger` every second is started in
the next lines.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_timer_driven_execution.c][cyclic trigger thread]-->
```c
pthread_t cyclicTriggerThread;
if (!createThread(&cyclicTriggerThread, cyclicTriggerCallback))
{
    printf("failed to create thread\n");
    return -1;
}
```

Everything is prepared and we enter the event loop. We start by gathering all
notifications in an array.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_timer_driven_execution.c][[event loop]]-->
```c
uint64_t missedElements = 0U;
uint64_t numberOfNotifications = 0U;

// array where all notifications from iox_ws_wait will be stored
iox_notification_info_t notificationArray[NUMBER_OF_NOTIFICATIONS];

while (keepRunning)
{
    numberOfNotifications = iox_ws_wait(waitSet, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedElements);
    // ...
}
```

The code checks for a notification from the `cyclicTrigger` and calls the attached
callback with `iox_notification_info_call(notification)`.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_timer_driven_execution.c][handle events]-->
```c
for (uint64_t i = 0U; i < numberOfNotifications; ++i)
{
    iox_notification_info_t notification = notificationArray[i];

    if (iox_notification_info_does_originate_from_user_trigger(notification, cyclicTrigger))
    {
        // call myCyclicRun
        iox_notification_info_call(notification);
    }
}
```

The last thing we have to do is to cleanup all the used resources.

<!--[geoffrey][iceoryx_examples/waitset_in_c/ice_c_waitset_timer_driven_execution.c][cleanup all resources]-->
```c
joinThread(cyclicTriggerThread);

waitSetSigHandlerAccess = NULL; // invalidate for signal handler
iox_ws_deinit(waitSet);

iox_user_trigger_deinit(cyclicTrigger);
```

<center>
[Check out waitset_in_c on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/waitset_in_c){ .md-button } <!--NOLINT github url required for website-->
</center>
