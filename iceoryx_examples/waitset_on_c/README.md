# WaitSet in C

## Introduction

A detailed introduction into the WaitSet topic can be found in the 
[waitset C++ example](../waitset). Here we will only introduce the C API and
not the WaitSet in general. For that we will take a look at the same use case
as the [waitset C++ example](../waitset). The examples are also structured 
in the same way as the C++ ones.

## Expected output

<!-- @todo Add expected output with asciinema recording before v1.0-->

## Code walkthrough

To run an example you need a running `iox-roudi` and the waitset publisher
`iox-ex-c-waitset-publisher`. They are identical to the ones introduced
in the [icedelivery C example](../icedelivery_on_c).

### Gateway
Let's say we would like to write a gateway and would like to forward every 
incoming message from a subscriber in the same manner. Like performing a 
memcopy of the received data into a specific struct.

This could be performed by a function which we attach to an event as a
callback. In our case we have the function `subscriberCallback` which 
prints out the subscriber pointer and the content of the received sample.
```c
void subscriberCallback(iox_sub_t const subscriber)
{
    const void* chunk;
    if (iox_sub_get_chunk(subscriber, &chunk))
    {
        printf("subscriber: %p received %u\n", subscriber, ((struct CounterTopic*)chunk)->counter);

        iox_sub_release_chunk(subscriber, chunk);
    }
}
```

After we registered our runtime we create some stack storage for our WaitSet,
initialize it and attach a `shutdownTrigger` to handle `CTRL-c`.
```c
iox_runtime_init("iox-c-ex-waitset-gateway");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

iox_ws_attach_user_trigger_event(waitSet, shutdownTrigger, 0U, NULL);
```

In the next steps we create 4 subscribers with `iox_sub_init`, 
subscribe them to our topic
and attach the event `SubscriberEvent_HAS_SAMPLES` to the WaitSet with
the `subscriberCallback` and an event id `1U`.
```c
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];

const uint64_t historyRequest = 1U;
const uint64_t queueCapacity = 256U;
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    iox_sub_t subscriber = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", queueCapacity, historyRequest);

    iox_sub_subscribe(subscriber);
    iox_ws_attach_subscriber_event(waitSet, subscriber, SubscriberEvent_HAS_SAMPLES, 1U, subscriberCallback);
}
```

Now that everything is set up we enter the event loop. It always starts with
a call to `iox_ws_wait`, a blocking call which returns us the number
of occurred events.
```c
uint64_t missedElements = 0U;
uint64_t numberOfEvents = 0U;

// array where all events from iox_ws_wait will be stored
iox_event_info_t eventArray[NUMBER_OF_EVENTS];

// event loop
bool keepRunning = true;
while (keepRunning)
{
    numberOfEvents =
        iox_ws_wait(waitSet, eventArray, NUMBER_OF_EVENTS, &missedElements);
```

The events which have occurred are stored in the `eventArray`. We iterate through
it, if the `shutdownTrigger` was evented we terminate the program otherwise
we call the callback with `iox_event_info_call(event)`.
```c
for (uint64_t i = 0U; i < numberOfEvents; ++i)
{
    iox_event_info_t event = eventArray[i];

    if (iox_event_info_does_originate_from_user_trigger(event, shutdownTrigger))
    {
        keepRunning = false;
    }
    else
    {
        iox_event_info_call(event);
    }
}
```

Before we can close the program we cleanup all resources.
```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    iox_sub_unsubscribe((iox_sub_t) & (subscriberStorage[i]));
    iox_sub_deinit((iox_sub_t) & (subscriberStorage[i]));
}

iox_ws_deinit(waitSet);
iox_user_trigger_deinit(shutdownTrigger);
```

### Grouping
In this scenario we have two groups of subscribers. We are interested in the
data of the first group and would like to print them onto the console and the
data of the second group should be discarded.

We start like in every example with creating the WaitSet and attaching the
`shutdownTrigger`.
```c
iox_runtime_init("iox-c-ex-waitset-grouping");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

iox_ws_attach_user_trigger_event(waitSet, shutdownTrigger, 0U, NULL);
```

After that we can create a list of subscribers and subscribe them to our topic.
```c
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBERS];
iox_sub_t subscriber[NUMBER_OF_SUBSCRIBERS];

const uint64_t historyRequest = 1U;
const uint64_t queueCapacity = 256U;
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriber[i] = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", queueCapacity, historyRequest);

    iox_sub_subscribe(subscriber[i]);
}
```

To distinct our two groups we set the eventId of the first group to
`123` and of the second group to `456`. The first two subscribers are attached with
the `Subscriber_HAS_SAMPLES` event and the event id of the first group to our waitset. 
The third and forth subscriber is attached to the same
waitset under the second group id.
```c
const uint64_t FIRST_GROUP_ID = 123;
const uint64_t SECOND_GROUP_ID = 456;

for (uint64_t i = 0U; i < 2U; ++i)
{
    iox_ws_attach_subscriber_event(waitSet, subscriber[i], SubscriberEvent_HAS_SAMPLES, FIRST_GROUP_ID, NULL);
}

for (uint64_t i = 2U; i < 4U; ++i)
{
    iox_ws_attach_subscriber_event(waitSet, subscriber[i], SubscriberEvent_HAS_SAMPLES, SECOND_GROUP_ID, NULL);
}
```

We are again ready for our event loop. We start as usual by setting the array
of events by calling `iox_ws_wait`.
```c
bool keepRunning = true;
while (keepRunning)
{
    numberOfEvents =
        iox_ws_wait(waitSet, eventArray, NUMBER_OF_EVENTS, &missedElements);
```

When we iterate through the array we handle the `shutdownTrigger` first.
We check if an event is from the first group by calling 
`iox_event_info_get_event_id` and compare the result with `FIRST_GROUP_ID`.
If that is the case we acquire the subscriber handle with
`iox_event_info_get_subscriber_origin`. This allows us to receive the new
sample and to print the result to the console.
The second group is handled in the same way. But we do not print the new samples
to screen, we just discard them.
```c
for (uint64_t i = 0U; i < numberOfEvents; ++i)
{
    iox_event_info_t event = eventArray[i];

    if (iox_event_info_does_originate_from_user_trigger(event, shutdownTrigger))
    {
        keepRunning = false;
    }
    else if (iox_event_info_get_event_id(event) == FIRST_GROUP_ID)
    {
        iox_sub_t subscriber = iox_event_info_get_subscriber_origin(event);
        const void* chunk;
        if (iox_sub_get_chunk(subscriber, &chunk))
        {
            printf("received: %u\n", ((struct CounterTopic*)chunk)->counter);

            iox_sub_release_chunk(subscriber, chunk);
        }
    }
    else if (iox_event_info_get_event_id(event) == SECOND_GROUP_ID)
    {
        printf("dismiss data\n");
        iox_sub_t subscriber = iox_event_info_get_subscriber_origin(event);
        iox_sub_release_queued_chunks(subscriber);
    }
}
```

The last thing we have to do is to cleanup all the acquired resources.
```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    iox_sub_unsubscribe((iox_sub_t) & (subscriberStorage[i]));
    iox_sub_deinit((iox_sub_t) & (subscriberStorage[i]));
}

iox_ws_deinit(waitSet);
iox_user_trigger_deinit(shutdownTrigger);
```

### Individual
We also can handle every event individualy. For instance if you would like
to have a different reaction for every subscriber which has received a sample.
One way would be to assign every subscriber a different callback, here we look
at a different approach. We check if the event originated from a specific 
subscriber and then perform the calls on that subscriber directly.

We start as usual, by creating a WaitSet and attach the `shutdownTrigger` to it.
```c
iox_runtime_init("iox-c-ex-waitset-individual");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

iox_ws_attach_user_trigger_event(waitSet, shutdownTrigger, 0U, NULL);
```

Now we create two subscriber, subscribe them to our topic and attach them to
the waitset without a callback and with the same trigger id.
```c
const uint64_t historyRequest = 1U;
const uint64_t queueCapacity = 256U;
subscriber[0] = iox_sub_init(&(subscriberStorage[0]), "Radar", "FrontLeft", "Counter", queueCapacity, historyRequest);
subscriber[1] = iox_sub_init(&(subscriberStorage[1]), "Radar", "FrontLeft", "Counter", queueCapacity, historyRequest);

iox_sub_subscribe(subscriber[0]);
iox_sub_subscribe(subscriber[1]);

iox_ws_attach_subscriber_event(waitSet, subscriber[0U], SubscriberEvent_HAS_SAMPLES, 0U, NULL);
iox_ws_attach_subscriber_event(waitSet, subscriber[1U], SubscriberEvent_HAS_SAMPLES, 0U, NULL);
```

We are ready to start the event loop. We begin by acquiring the array of all
the triggered triggers.
```c
bool keepRunning = true;
while (keepRunning)
{
    numberOfEvents =
        iox_ws_wait(waitSet, eventArray, NUMBER_OF_EVENTS, &missedElements);
```

The `shutdownTrigger` is handled as usual and
we use `iox_event_info_does_originate_from_subscriber` 
to identify the event that originated from a specific subscriber. If it originated
from the first subscriber we print the received data to the console, if it 
originated from the second subscriber we discard the data.
```c
    for (uint64_t i = 0U; i < numberOfEvents; ++i)
    {
        iox_event_info_t event = eventArray[i];

        if (iox_event_info_does_originate_from_user_trigger(event, shutdownTrigger))
        {
            keepRunning = false;
        }
        else if (iox_event_info_does_originate_from_subscriber(event, subscriber[0]))
        {
            const void* chunk;
            if (iox_sub_get_chunk(subscriber[0], &chunk))
            {
                printf("subscriber 1 received: %u\n", ((struct CounterTopic*)chunk)->counter);

                iox_sub_release_chunk(subscriber[0], chunk);
            }
        }
        else if (iox_event_info_does_originate_from_subscriber(event, subscriber[1]))
        {
            iox_sub_release_queued_chunks(subscriber[1]);
            printf("subscriber 2 received something - dont care\n");
        }
    }
```
We conclude the example as always, be cleaning up the resources.

```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    iox_sub_unsubscribe((iox_sub_t) & (subscriberStorage[i]));
    iox_sub_deinit((iox_sub_t) & (subscriberStorage[i]));
}

iox_ws_deinit(waitSet);
iox_user_trigger_deinit(shutdownTrigger);
```

### Sync
In this example we demonstrate how you can use the WaitSet to trigger a cyclic
call every second. We use a user trigger which will be triggered in a separate
thread every second to signal the WaitSet that its time for the next run.
Additionally, we attach a callback (`cyclicRun`) to this user trigger
so that the event can directly call the cyclic call.

We begin by creating the waitset and attach the `shutdownTrigger`.
```c
iox_runtime_init("iox-c-ex-waitset-sync");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

iox_ws_attach_user_trigger_event(waitSet, shutdownTrigger, 0, NULL);
```

Now we create our cyclic trigger and attach it to our waitset with a eventId
of `0` and the callback `cyclicRun`.
```c
cyclicTrigger = iox_user_trigger_init(&cyclicTriggerStorage);
iox_ws_attach_user_trigger_event(waitSet, cyclicTrigger, 0, cyclicRun);
```

The thread which will trigger the `cyclicTrigger` every second is started in
the next lines.
```c
pthread_t cyclicTriggerThread;
if (pthread_create(&cyclicTriggerThread, NULL, cyclicTriggerCallback, NULL))
{
    printf("failed to create thread\n");
    return -1;
}
```

Everything is prepared and we enter the event loop. We start by gathering all
events in an array.
```c
while (keepRunning)
{
    numberOfEvents =
        iox_ws_wait(waitSet, eventArray, NUMBER_OF_EVENTS, &missedElements);
```

The `shutdownTrigger` is handled as usual and the `cyclicTrigger` is handled by
just calling the attached callback with `iox_event_info_call(event)`.
```c
    for (uint64_t i = 0U; i < numberOfEvents; ++i)
    {
        iox_event_info_t event = eventArray[i];

        if (iox_event_info_does_originate_from_user_trigger(event, shutdownTrigger))
        {
            // CTRL+c was pressed -> exit
            keepRunning = false;
        }
        else
        {
            // call myCyclicRun
            iox_event_info_call(event);
        }
    }
```

The last thing we have to do is to cleanup all the used resources.
```c
    pthread_join(cyclicTriggerThread, NULL);
    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownTrigger);
```
