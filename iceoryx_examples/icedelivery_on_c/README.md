# icedelivery in C

You can find a more detailled description of the C API in the [iceoryx_binding_c README.md](../../iceoryx_binding_c)

## Introduction

The behavior and structure is identical to the [icedelivery C++ example](../icedelivery/)
so that we explain here only the C API differences and not the
underlying mechanisms.

## Expected output

<!-- @todo Add expected output with asciinema recording before v1.0-->

## Code walkthrough

### Subscriber

Like in the
[icedelivery C++ example](../icedelivery/)
we again follow the steps like:

 1. Create runtime instance.
 2. Create subscriber port.
 3. Subscribe to the offered service
 4. Receive data
 5. Unsubscribe.
 6. **C API: Additionally, we have to remove the previously allocated Subscriber
        port!**

Let's take a look at the `receiving` function which comes with the
`ice_c_subscriber.c` example.

 1. We register our process at roudi with the name `iox-c-subscriber`
    ```c
    iox_runtime_init("iox-c-subscriber");
    ```
  
 2. We create a subscriber port and are subscribing to the service
    {"Radar", "FrontLeft", "Counter" }. Hereby the `historyRequest`
    tells the subscriber how many previously send samples it should receive
    right after the connection is established and the `queueCapacity` how many
    samples the subscriber can hold. These are samples which the publisher has
    send before the subscriber was connected.
    The `subscriberStorage` is the place where the subscriber is stored in
    memory and `subscriber` is actually a pointer to that location.
    ```c
    const uint64_t historyRequest = 10U;
    const uint64_t queueCapacity = 5U;
    iox_sub_storage_t subscriberStorage;
    iox_sub_t subscriber = iox_sub_init(&subscriberStorage, "Radar", "FrontLeft", "Object", queueCapacity, historyRequest);
    ```

  3. We subscribe to the service.
     ```c
     iox_sub_subscribe(subscriber);
     ```

  4. In this loop we receive samples as long the `killswitch` is not
     set to `true` by an external signal and then print the counter
     value to the console.
     ```c
     while (!killswitch)
     {
         if (SubscribeState_SUBSCRIBED == iox_sub_get_subscription_state(subscriber))
         {
             const void* chunk = NULL;
             while (ChunkReceiveError_SUCCESS == iox_sub_get_chunk(subscriber, &chunk))
             {
                 const struct RadarObject* sample = (const struct RadarObject*)(chunk);
                 printf("Got value: %.0f\n", sample->x);
                 iox_sub_release_chunk(subscriber, chunk);
             }
         }
         else
         {
             printf("Not subscribed!\n");
         }

         sleep_for(1000);
     }
     ```
  
  5. After we stop receiving samples we would like to unsubscribe.
     ```c
     iox_sub_unsubscribe(subscriber);
     ```

  6. When using the C API we have to cleanup the subscriber after
     its usage.
     ```c
     iox_sub_deinit(subscriber);
     ```

### Publisher
The publisher is implemented in a way like in the
[icedelivery C++ example](../icedelivery/).

 1. Create runtime instance.
 2. Create publisher port.
 3. Offer the service
 4. Send data
 5. Stop offering the service
 6. **C API: Additionally, we have to remove the previously allocated Publisher
        port!**

Let's take a look at the `sending` function which comes with the
`ice_c_publisher.c` example.

 1. We register our process at roudi with the name `iox-c-subscriber`
    ```c
    iox_runtime_init("iox-c-publisher");
    ```
 2. We create a publisher with the service
    {"Radar", "FrontLeft", "Counter"}
    ```c
    const uint64_t historyRequest = 10U;
    iox_pub_storage_t publisherStorage;
    iox_pub_t publisher = iox_pub_init(&publisherStorage, "Radar", "FrontLeft", "Object", historyRequest);
    ```
 3. We offer our service to the world.
    ```c
    iox_pub_offer(publisher);
    ```

 4. Till an external signal sets `killswitch` to `true` we will send an
    incrementing number to all subscribers every send and print the
    value of this number to the console.
    ```c
    double ct = 0.0;

    while (!killswitch)
    {
        void* chunk = NULL;
        if (AllocationResult_SUCCESS == iox_pub_allocate_chunk(publisher, &chunk, sizeof(struct RadarObject)))
        {
            struct RadarObject* sample = (struct RadarObject*)chunk;

            sample->x = ct;
            sample->y = ct;
            sample->z = ct;

            printf("Sent value: %.0f\n", ct);

            iox_pub_send_chunk(publisher, chunk);

            ++ct;

            sleep_for(400);
        }
        else
        {
            printf("Failed to allocate chunk!");
        }
    }
    ```

 5. We stop offering our service.
    ```c
    iox_pub_stop_offer(publisher);
    ```

 6. And we cleanup our publisher port.
    ```c
    iox_pub_destroy(publisher);
    ```
