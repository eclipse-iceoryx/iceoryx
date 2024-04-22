# icedelivery in C

You can find a more detailed description of the C API in the
[iceoryx_binding_c README.md](../../iceoryx_binding_c/README.md).

## Introduction

The behavior and structure is identical to the
[icedelivery C++ example](../icedelivery)
so that we explain here only the C API differences and not the underlying mechanisms.

## Expected Output

[![asciicast](https://asciinema.org/a/407361.svg)](https://asciinema.org/a/407361)

## Code walkthrough

### Subscriber

As in the
[icedelivery C++ example](../icedelivery),
we perform the following steps:

 1. Create a runtime instance.
 2. Create a subscriber with some options.
 3. Receive data.
 4. **C API: Additionally, we have to remove the previously allocated subscriber
        port!**

Let's take a look at the `receiving` function that comes with the
`ice_c_subscriber.c` example.

 1. We register our process at RouDi with the name `iox-c-subscriber`

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_subscriber.c][create runtime instance]-->
```c
const char APP_NAME[] = "iox-c-subscriber";
iox_runtime_init(APP_NAME);
```

 2. We create a subscriber with the service description
    {"Radar", "FrontLeft", "Object" }. We also set subscriber options. The
    `historyRequest` tells the subscriber how many previously sent samples it
    shall request from all offered and matching publishers and the
    `queueCapacity` how many unread samples the subscriber can queue. The
    `nodeName` is the name of the node the subscriber is associated with.
    The `subscriberStorage` is the place where the subscriber is stored in
    memory and `subscriber` is actually a pointer to that location.

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_subscriber.c][create subscriber port]-->
```c
iox_sub_options_t options;
iox_sub_options_init(&options);
options.historyRequest = 10U;
options.queueCapacity = 50U;
options.nodeName = "iox-c-subscriber-node";
iox_sub_storage_t subscriberStorage;

iox_sub_t subscriber = iox_sub_init(&subscriberStorage, "Radar", "FrontLeft", "Object", &options);
```

 3. We receive samples in a loop and print the received data on the console as
    long as the `keepRunning` is not set to `false` by an external signal.

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_subscriber.c][receive and print data]-->
```c
while (keepRunning)
{
    if (SubscribeState_SUBSCRIBED == iox_sub_get_subscription_state(subscriber))
    {
        const void* userPayload = NULL;
        // we will receive more than one sample here since the publisher is sending a
        // new sample every 400 ms and we check for new samples only every second
        while (ChunkReceiveResult_SUCCESS == iox_sub_take_chunk(subscriber, &userPayload))
        {
            const struct RadarObject* sample = (const struct RadarObject*)(userPayload);
            printf("%s got value: %.0f\n", APP_NAME, sample->x);
            fflush(stdout);
            iox_sub_release_chunk(subscriber, userPayload);
        }
        printf("\n");
    }
    else
    {
        printf("Not subscribed!\n");
    }

    sleep_for(1000);
}
```

 4. When using the C API we have to clean up the subscriber after
    its usage.

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_subscriber.c][cleanup]-->
```c
iox_sub_deinit(subscriber);
```

### Publisher

The publisher is implemented in a similar way like in the
[icedelivery C++ example](../icedelivery):

 1. Create a runtime instance.
 2. Create a publisher with some options.
 3. Send data.
 4. **C API: Additionally, we have to remove the previously allocated publisher
        port!**

Let's take a look at the `sending` function that comes with the
`ice_c_publisher.c` example.

 1. We register our process at RouDi with the name `iox-c-publisher`

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_publisher.c][create runtime instance]-->
```c
const char APP_NAME[] = "iox-c-publisher";
iox_runtime_init(APP_NAME);
```

 2. We create a publisher with the service description
    {"Radar", "FrontLeft", "Object"}

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_publisher.c][create publisher port]-->
```c
iox_pub_options_t options;
iox_pub_options_init(&options);
options.historyCapacity = 10U;
options.nodeName = "iox-c-publisher-node";
iox_pub_storage_t publisherStorage;
iox_pub_t publisher = iox_pub_init(&publisherStorage, "Radar", "FrontLeft", "Object", &options);
```

 3. Until an external signal sets `keepRunning` to `false`, we will send an
    incrementing number to all subscribers in every iteration and print the
    value of that number to the console.

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_publisher.c][send and print number]-->
```c
double ct = 0.0;

while (keepRunning)
{
    void* userPayload = NULL;
    if (AllocationResult_SUCCESS == iox_pub_loan_chunk(publisher, &userPayload, sizeof(struct RadarObject)))
    {
        struct RadarObject* sample = (struct RadarObject*)userPayload;

        sample->x = ct;
        sample->y = ct;
        sample->z = ct;

        printf("%s sent value: %.0f\n", APP_NAME, ct);
        fflush(stdout);

        iox_pub_publish_chunk(publisher, userPayload);

        ++ct;

        sleep_for(400);
    }
    else
    {
        printf("Failed to allocate chunk!");
    }
}
```

 4. And we clean up our publisher port.

<!--[geoffrey][iceoryx_examples/icedelivery_in_c/ice_c_publisher.c][cleanup]-->
```c
iox_pub_deinit(publisher);
```

<center>
[Check out icedelivery_in_c on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/icedelivery_in_c){ .md-button } <!--NOLINT github url for website-->
</center>
