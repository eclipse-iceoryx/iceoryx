# WaitSet C binding

A detailed introduction into the WaitSet topic can be found in the 
[waitset C++ example](../waitset). Here we will only introduce the C API and
not the WaitSet in general. For that we will take a look at the same use case
as the [waitset C++ example](../waitset). The examples are also structured 
in the same way as the C++ ones.

## Examples

<!-- @todo Add expected output with asciinema recording before v1.0-->

To run an example you need a running `iox-roudi` and the waitset publisher
`iox-ex-c-waitset-publisher`. They are identical to the ones introduced
in the [icedelivery C example](../icedelivery_on_c).

### Gateway
Lets say we would like to write a gateway and would like to forward every 
incoming message from a subscriber in the same manner. Like performing a 
memcopy of the received data into a specific struct.

This could be performed by a function which we attach to a trigger as a
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

iox_user_trigger_attach_to_waitset(shutdownTrigger, waitSet, 0, NULL);
```

During the next step we create 4 subscribers with `iox_sub_init`, 
subscribe them to our topic
and attach the event `SubscriberEvent_HAS_NEW_SAMPLES` to the WaitSet with
the `subscriberCallback`.
```c
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBER];

uint64_t historyRequest = 1U;
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
{
    iox_sub_t subscriber = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", historyRequest);

    iox_sub_subscribe(subscriber, 256);
    iox_sub_attach_to_waitset(subscriber, waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 1, subscriberCallback);
}
```

Now that everything is set up we enter the event loop. It always starts with
a call to `iox_ws_wait` which is a blocking call which returns us the number
of triggers which had signalled an event.
```c
uint64_t missedElements = 0U;
uint64_t numberOfTriggeredConditions = 0U;

// array where all trigger from iox_ws_wait will be stored
iox_trigger_info_storage_t triggerArray[NUMBER_OF_TRIGGER];

// event loop
bool keepRunning = true;
while (keepRunning)
{
    numberOfTriggeredConditions =
        iox_ws_wait(waitSet, (iox_trigger_info_t)triggerArray, NUMBER_OF_TRIGGER, &missedElements);
```

The triggered Triggers are contained in the `triggerArray`. We iterate through
it, if the `shutdownTrigger` was triggered we terminate the program otherwise
we call the callback with `iox_trigger_info_call(trigger)`.
```c
for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
{
    iox_trigger_info_t trigger = (iox_trigger_info_t) & (triggerArray[i]);

    if (iox_trigger_info_does_originate_from_user_trigger(trigger, shutdownTrigger))
    {
        keepRunning = false;
    }
    else
    {
        iox_trigger_info_call(trigger);
    }
}
```

Before we can close the program we cleanup all resources.
```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
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
date of the second group should be discarded.

We start like in every example with creating the WaitSet and attaching the
`shutdownTrigger`.
```c
iox_runtime_init("iox-c-ex-waitset-grouping");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

iox_user_trigger_attach_to_waitset(shutdownTrigger, waitSet, 0, NULL);
```

After that we can create a list of subscribers and subscribe them to our topic.
```c
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBER];
iox_sub_t subscriber[NUMBER_OF_SUBSCRIBER];

uint64_t historyRequest = 1U;
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
{
    subscriber[i] = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", historyRequest);

    iox_sub_subscribe(subscriber[i], 256);
}
```

To distinct our two groups we create set the triggerId of the first group to
`123` and of the second group to `456`. For the first two subscribers we attach
the `Subscriber_HAS_NEW_SAMPLES` event to our waitset with the trigger id of
the first group. The third and forth subscriber is  attached to the same
waitset under the second group id.
```c
const uint64_t FIRST_GROUP_ID = 123;
const uint64_t SECOND_GROUP_ID = 456;

for (uint64_t i = 0U; i < 2U; ++i)
{
    iox_sub_attach_to_waitset(subscriber[i], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, FIRST_GROUP_ID, NULL);
}

for (uint64_t i = 2U; i < 4U; ++i)
{
    iox_sub_attach_to_waitset(subscriber[i], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, SECOND_GROUP_ID, NULL);
}
```

We are again ready for our event loop. We start as usual by setting the array
of triggers by calling `iox_ws_wait`.
```c
bool keepRunning = true;
while (keepRunning)
{
    numberOfTriggeredConditions =
        iox_ws_wait(waitSet, (iox_trigger_info_t)triggerArray, NUMBER_OF_TRIGGER, &missedElements);
```

When we iterate through the array we handle the `shutdownTrigger` first.
We check if a trigger is from the first group by calling 
`iox_trigger_info_get_trigger_id` and compare the result with `FIRST_GROUP_ID`.
If that is the case we acquire the subscriber handle with
`iox_trigger_info_get_subscriber_origin`. This allows us to receive the new
sample and to print the result to the console.
The second group is handled in the same way. But we do not print the new samples
to screen, we just discard them.
```c
for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
{
    iox_trigger_info_t trigger = (iox_trigger_info_t) & (triggerArray[i]);

    if (iox_trigger_info_does_originate_from_user_trigger(trigger, shutdownTrigger))
    {
        keepRunning = false;
    }
    else if (iox_trigger_info_get_trigger_id(trigger) == FIRST_GROUP_ID)
    {
        iox_sub_t subscriber = iox_trigger_info_get_subscriber_origin(trigger);
        const void* chunk;
        if (iox_sub_get_chunk(subscriber, &chunk))
        {
            printf("received: %u\n", ((struct CounterTopic*)chunk)->counter);

            iox_sub_release_chunk(subscriber, chunk);
        }
    }
    else if (iox_trigger_info_get_trigger_id(trigger) == SECOND_GROUP_ID)
    {
        printf("dismiss data\n");
        iox_sub_t subscriber = iox_trigger_info_get_subscriber_origin(trigger);
        iox_sub_release_queued_chunks(subscriber);
    }
}
```

The last thing we have to do is to cleanup all the acquired resources.
```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
{
    iox_sub_unsubscribe((iox_sub_t) & (subscriberStorage[i]));
    iox_sub_deinit((iox_sub_t) & (subscriberStorage[i]));
}

iox_ws_deinit(waitSet);
iox_user_trigger_deinit(shutdownTrigger);
```

### Individual
We also can handle every trigger individualy. For instance if you would like
to have a different reaction for every subscriber which has received a sample.
One way would be to assign every subscriber a different callback, here we look
at a different approach.

We start as usual, by creating a WaitSet and attach the `shutdownTrigger` to it.
```c
iox_runtime_init("iox-c-ex-waitset-individual");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

iox_user_trigger_attach_to_waitset(shutdownTrigger, waitSet, 0, NULL);
```

Now we create two subscriber, subscribe them to our topic and attach them to
the waitset without a callback and with the same trigger id.
```c
uint64_t historyRequest = 1U;
subscriber[0] = iox_sub_init(&(subscriberStorage[0]), "Radar", "FrontLeft", "Counter", historyRequest);
subscriber[1] = iox_sub_init(&(subscriberStorage[1]), "Radar", "FrontLeft", "Counter", historyRequest);

iox_sub_subscribe(subscriber[0], 256);
iox_sub_subscribe(subscriber[1], 256);

iox_sub_attach_to_waitset(subscriber[0], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 0, NULL);
iox_sub_attach_to_waitset(subscriber[1], waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 0, NULL);
```

We are ready to start the event loop. We begin with acquiring the array of all
the triggered triggers.
```c
bool keepRunning = true;
while (keepRunning)
{
    numberOfTriggeredConditions =
        iox_ws_wait(waitSet, (iox_trigger_info_t)triggerArray, NUMBER_OF_TRIGGER, &missedElements);
```

The `shutdownTrigger` is handled as usual and
we use `iox_trigger_info_does_originate_from_subscriber` 
to identify the trigger that originated from a specific subscriber. If it originated
from the first subscriber we print the received data to the console, if it 
originated from the second subscriber we discard the data.
```c
    for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
    {
        iox_trigger_info_t trigger = (iox_trigger_info_t) & (triggerArray[i]);

        if (iox_trigger_info_does_originate_from_user_trigger(trigger, shutdownTrigger))
        {
            keepRunning = false;
        }
        else if (iox_trigger_info_does_originate_from_subscriber(trigger, subscriber[0]))
        {
            const void* chunk;
            if (iox_sub_get_chunk(subscriber[0], &chunk))
            {
                printf("subscriber 1 received: %u\n", ((struct CounterTopic*)chunk)->counter);

                iox_sub_release_chunk(subscriber[0], chunk);
            }
        }
        else if (iox_trigger_info_does_originate_from_subscriber(trigger, subscriber[1]))
        {
            iox_sub_release_queued_chunks(subscriber[1]);
            printf("subscriber 2 received something - dont care\n");
        }
    }
```
We conclude the example as always, be cleaning up the resources.

```c
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
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
so that the trigger can directly call the cyclic call.

We begin by creating the waitset and attach the `shutdownTrigger`.
```c
iox_runtime_init("iox-c-ex-waitset-sync");

iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownTrigger = iox_user_trigger_init(&shutdownTriggerStorage);

iox_user_trigger_attach_to_waitset(shutdownTrigger, waitSet, 0, NULL);
```

Now we create our cyclic trigger and attach it to our waitset with a triggerId
of `0` and the callback `cyclicRun`.
```c
cyclicTrigger = iox_user_trigger_init(&cyclicTriggerStorage);
iox_user_trigger_attach_to_waitset(cyclicTrigger, waitSet, 0, cyclicRun);
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
triggered Triggers in an array.
```c
while (keepRunning)
{
    numberOfTriggeredConditions =
        iox_ws_wait(waitSet, (iox_trigger_info_t)triggerArray, NUMBER_OF_TRIGGER, &missedElements);
```

The `shutdownTrigger` is handled as usual and the `cyclicTrigger` is handled by
just calling the attached callback with `iox_trigger_info_call(trigger)`.
```c
    for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
    {
        iox_trigger_info_t trigger = (iox_trigger_info_t) & (triggerArray[i]);

        if (iox_trigger_info_does_originate_from_user_trigger(trigger, shutdownTrigger))
        {
            // CTRL+c was pressed -> exit
            keepRunning = false;
        }
        else
        {
            // call myCyclicRun
            iox_trigger_info_call(trigger);
        }
    }
```

The last thing we have to do is to cleanup all the used resources.
```c
    pthread_join(cyclicTriggerThread, NULL);
    iox_ws_deinit(waitSet);
    iox_user_trigger_deinit(shutdownTrigger);
```
