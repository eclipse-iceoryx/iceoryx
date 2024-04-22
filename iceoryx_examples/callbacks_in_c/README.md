# Listener in C (or how to use callbacks with iceoryx)

## Thread Safety

The Listener is thread-safe and can be used without restrictions.
But be aware that all provided callbacks are executed concurrently
in the background thread of the Listener. If you access structures
inside this callback you have to either ensure that you are the only
one accessing it or that it is accessed with a guard like a `mutex`.

## Introduction

For a general introduction into the Listener concept please take a look at
the first part of the
[Listener C++ example](../callbacks)
and at the Glossary of the
[WaitSet C++ example](../waitset).

## Expected Output

[![asciicast](https://asciinema.org/a/407369.svg)](https://asciinema.org/a/407369)

## Code Walkthrough

!!! attention
    Please be aware about the thread-safety restrictions of the _Listener_ and
    read the [Thread Safety](#thread-safety) chapter carefully.

The C version of the callbacks example performs the identical tasks as the
C++ version. We have again an application which offers two services called
`Radar.FrontLeft.Counter` and `Radar.FrontRight.Counter`. Every time we have
received a sample from each service we calculate the sum of it.

### ice_c_callbacks_publisher.c

The publisher contains only already known iceoryx features. If some of them
are not known to you please take a look at the
[icedelivery in C example](../icedelivery_in_c).

### ice_c_callbacks_subscriber.c

#### int main()

The subscriber starts as usual by registering the process at the runtime.
In the next step, we set up some `listenerStorage` and initialize the listener which will
start a background thread for the upcoming event-triggered callbacks.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][create listener]-->
```c
iox_listener_storage_t listenerStorage;
iox_listener_t listener = iox_listener_init(&listenerStorage);
```

Besides the subscribers we also would like to have an event that will be triggered
by our self - the `heartbeat`.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][create heartbeat]-->
```c
iox_user_trigger_storage_t heartbeatStorage;
heartbeat = iox_user_trigger_init(&heartbeatStorage);
```

Both subscribers use the same options which we set up with:

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][set subscriber options]-->
```c
iox_sub_options_t options;
iox_sub_options_init(&options);
options.historyRequest = 10U;
options.queueCapacity = 50U;
options.nodeName = "iox-c-callback-subscriber-node";
```

and then we can construct the two subscribers `subscriberLeft` and `subscriberRight`.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][create subscribers]-->
```c
iox_sub_t subscriberLeft = iox_sub_init(&subscriberLeftStorage, "Radar", "FrontLeft", "Counter", &options);
iox_sub_t subscriberRight = iox_sub_init(&subscriberRightStorage, "Radar", "FrontRight", "Counter", &options);
```

Now that everything is initialized, we start our `heartbeatTriggerThread` which
triggers our `heartbeat` every 4 seconds.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][send a heartbeat every 4 seconds]-->
```c
pthread_t heartbeatTriggerThread;
if (pthread_create(&heartbeatTriggerThread, NULL, cyclicHeartbeatTrigger, NULL))
{
    printf("failed to create thread\n");
    return -1;
}
```

Attaching the subscribers and the heartbeat allows the Listener to call the callbacks
whenever the event is signaled by the _EventOrigin_.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][attach everything to the listener]-->
```c
// from here on the callbacks are called when an event occurs
iox_listener_attach_user_trigger_event(listener, heartbeat, &heartbeatCallback);
iox_listener_attach_subscriber_event(
    listener, subscriberLeft, SubscriberEvent_DATA_RECEIVED, &onSampleReceivedCallback);
iox_listener_attach_subscriber_event(
    listener, subscriberRight, SubscriberEvent_DATA_RECEIVED, &onSampleReceivedCallback);
```

A user trigger can emit only one event therefore we do not provide the event type as
an argument in the user trigger attach call.

Since we are following a push-based approach, i.e. without an event loop that is pulling
the events and processing them, we require a blocking call that waits until the process is
signaled to terminate.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][wait until someone presses CTRL+C]-->
```c
while (keepRunning)
{
    sleep_for(100);
}
```

When `keepRunning` is set to false, we clean up all the resources. First, we detach
the events from the Listener. This is an optional step since the Listener detaches
all events by itself when it is deinitialized. This applies also for all the _EventOrigins_,
if you for instance deinitialize an attached subscriber it will automatically detach
itself from the Listener.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][optional detachEvent, but not required]-->
```c
iox_listener_detach_user_trigger_event(listener, heartbeat);
iox_listener_detach_subscriber_event(listener, subscriberLeft, SubscriberEvent_DATA_RECEIVED);
iox_listener_detach_subscriber_event(listener, subscriberRight, SubscriberEvent_DATA_RECEIVED);
```

In a last step we have to release all acquired resources

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][cleanup]-->
```c
iox_user_trigger_deinit(heartbeat);
iox_sub_deinit(subscriberLeft);
iox_sub_deinit(subscriberRight);
iox_listener_deinit(listener);
```

#### The callbacks

Every callback must have a signature like `void (iox_event_origin_t)`. Our
`heartbeatCallback` just prints the message `heartbeat received` onto the console.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][heartbeat callback]-->
```c
void heartbeatCallback(iox_user_trigger_t userTrigger)
{
    (void)userTrigger;
    printf("heartbeat received\n");
    fflush(stdout);
}
```

The `onSampleReceivedCallback` is a little bit more complex. First we acquire all
the chunks and also have to find out which subscriber received the chunk. For that
we acquire the service description of the subscriber and if its instance equals
`FrontLeft` we store the chunk value in the `leftCache` otherwise in the `rightCache`.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][[subscriber callback][get data]]-->
```c
void onSampleReceivedCallback(iox_sub_t subscriber)
{
    // take all samples from the subscriber queue
    const struct CounterTopic* userPayload;
    while (iox_sub_take_chunk(subscriber, (const void**)&userPayload) == ChunkReceiveResult_SUCCESS)
    {
        iox_service_description_t serviceDescription = iox_sub_get_service_description(subscriber);
        if (strcmp(serviceDescription.instanceString, "FrontLeft") == 0)
        {
            leftCache.value = *userPayload;
            leftCache.isSet = true;
        }
        else if (strcmp(serviceDescription.instanceString, "FrontRight") == 0)
        {
            rightCache.value = *userPayload;
            rightCache.isSet = true;
        }
        printf("received: %d\n", userPayload->counter);
        fflush(stdout);
    }
    // ...
}
```

If both caches are set, we can calculate the sum of both chunks and print them to
the console. To start fresh in the next cycle, we reset the `leftCache` and
the `rightCache` afterward.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_subscriber.c][[subscriber callback][process data]]-->
```c
void onSampleReceivedCallback(iox_sub_t subscriber)
{
    // ...
    if (leftCache.isSet && rightCache.isSet)
    {
        printf("Received samples from FrontLeft and FrontRight. Sum of %d + %d = %d\n",
               leftCache.value.counter,
               rightCache.value.counter,
               leftCache.value.counter + rightCache.value.counter);
        fflush(stdout);
        leftCache.isSet = false;
        rightCache.isSet = false;
    }
}
```

### Additional context data for callbacks (ice_c_callbacks_with_context_data.c)

Sometimes we would like to modify data structures which are not globally available
within the callback. To facilitate this we provide the functions called
`iox_listener_attach_***_event_with_context_data` which allow to provide an
additional void pointer for the callback as second argument.

The following example is a simplified version of the
[ice_c_callbacks_subscriber.c](#ice_c_callbacks_subscriber.c) example where we
removed the cyclic heartbeat trigger. The key difference is that we have
a local variable called `counterService` in which we store the `leftCache`
and `rightCache` and we let the callback update that variable directly.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_with_context_data.c][local variable for caches]-->
```c
CounterService counterService;
counterService.leftCache.isSet = false;
counterService.rightCache.isSet = false;
```

The callback takes an additional void pointer argument which we cast then to
our CounterService to perform the same tasks as in the previous example but now
on `CounterService * self`.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_with_context_data.c][[subscriber callback][context data]]-->
```c
void onSampleReceivedCallback(iox_sub_t subscriber, void* contextData)
{
    if (contextData == NULL)
    {
        fprintf(stderr, "aborting onSampleReceivedCallback since contextData is a null pointer\n");
        return;
    }

    CounterService* self = (CounterService*)contextData;
    // ...
}
```

Finally, we have to attach both subscribers and provide the pointer to `counterService`
as additional argument so that we can access it in the callback.

!!! attention
    The user has to ensure that the contextData (`&counterService`) in
    `iox_listener_attach_subscriber_event_with_context_data`
    lives as long as the attachment with its callback is attached, otherwise
    the callback context data pointer is dangling.

<!--[geoffrey][iceoryx_examples/callbacks_in_c/ice_c_callbacks_with_context_data.c][attach everything to the listener]-->
```c
iox_listener_attach_subscriber_event_with_context_data(
    listener, subscriberLeft, SubscriberEvent_DATA_RECEIVED, &onSampleReceivedCallback, &counterService);
iox_listener_attach_subscriber_event_with_context_data(
    listener, subscriberRight, SubscriberEvent_DATA_RECEIVED, &onSampleReceivedCallback, &counterService);
```

<center>
[Check out callbacks_in_c on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/callbacks_in_c){ .md-button } <!--NOLINT github url required for website-->
</center>
