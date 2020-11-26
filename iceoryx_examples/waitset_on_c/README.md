# WaitSet C binding

A detailed introduction into the WaitSet topic can be found in the 
[waitset C++ example](../waitset). Here we will only introduce the C API and
not the WaitSet in general. For that we will take a look at the same use case
as the [waitset C++ example](../waitset). The examples are also structured 
in the same way as the C++ ones.

## Examples

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
initialize it and attach a `shutdownGuard` to handle `CTRL-c`.
```c
iox_ws_storage_t waitSetStorage;
iox_ws_t waitSet = iox_ws_init(&waitSetStorage);
shutdownGuard = iox_user_trigger_init(&shutdowGuardStorage);

iox_user_trigger_attach_to_ws(shutdownGuard, waitSet, 0, NULL);
```

During the next step we create 4 subscribers, subscribe them to our service
and attach the event `SubscriberEvent_HAS_NEW_SAMPLES` to the WaitSet with
the `subscriberCallback`.
```c
iox_sub_storage_t subscriberStorage[NUMBER_OF_SUBSCRIBER];

uint64_t historyRequest = 1U;
for (uint64_t i = 0U; i < NUMBER_OF_SUBSCRIBER; ++i)
{
    iox_sub_t subscriber = iox_sub_init(&(subscriberStorage[i]), "Radar", "FrontLeft", "Counter", historyRequest);

    iox_sub_subscribe(subscriber, 256);
    iox_sub_attach_to_ws(subscriber, waitSet, SubscriberEvent_HAS_NEW_SAMPLES, 1, subscriberCallback);
}
```

Now that everything is set up we enter the event loop. It always starts with
a call to `iox_ws_wait` which is a blocking call which returns us the number
of triggers which had signalled an event.
```c
uint64_t missedElements = 0U;
uint64_t numberOfTriggeredConditions = 0U;

// array where all trigger from iox_ws_wait will be stored
iox_trigger_state_storage_t triggerArray[NUMBER_OF_TRIGGER];

// event loop
bool keepRunning = true;
while (keepRunning)
{
    numberOfTriggeredConditions =
        iox_ws_wait(waitSet, (iox_trigger_state_t)triggerArray, NUMBER_OF_TRIGGER, &missedElements);
```

The triggered Triggers are contained in the `triggerArray`. We iterate through
it, if the `shutdownGuard` was triggered we terminate the program otherwise
we call the callback with `iox_trigger_state_call(trigger)`.
```c
for (uint64_t i = 0U; i < numberOfTriggeredConditions; ++i)
{
    iox_trigger_state_t trigger = (iox_trigger_state_t) & (triggerArray[i]);

    if (iox_trigger_state_does_originate_from_user_trigger(trigger, shutdownGuard))
    {
        keepRunning = false;
    }
    else
    {
        iox_trigger_state_call(trigger);
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
iox_user_trigger_deinit(shutdownGuard);
```