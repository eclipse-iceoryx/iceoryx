# [NOT YET IMPLEMENTED] icedelivery in C - Transfer data between POSIX applications

**Hint** This example shows a work in progress. The API for the `publisher` and 
`subscriber` is finished but we still have to integrate the new building blocks
into RouDi. Till then whis C API will not work.

You can find a more detailled description of the C API in the [iceoryx_binding_c README.md](../../iceoryx_binding_c)

## The icedelivery example

The behavior and structure is identical to the [icedelivery C++ example](../icedelivery/) 
so that we explain here only the C API differences and not the 
underlying mechanisms.

### Subscriber

Like in the 
[icedelivery C++ example](../icedelivery/)
we again follow the steps like:
 
 1. Create runtime instance.
 2. Create subscriber port.
 3. Subscribe to the offered service 
 4. Receive data 
 5. Unsubscribe.
 6. **C API: Additionally, we have to remove the Subscriber port!**

Let's take a look at the `receiving` function which comes with the
`ice_c_subscriber.c` example.

 1. We register our process at roudi with the name `iox-c-subscriber`
    ```c
    PoshRuntime_getInstance("/iox-c-subscriber");
    ```
  
 2. We create a subscriber port and are subscribing to the service 
    {"Radar", "FrontLeft", "Counter" }. Hereby the `historyRequest` 
    tells the subscriber how many samples it should receive right 
    after the connection (for the experts: field behavior). These are
    samples which the publisher has send before the subscriber was 
    connected.
    ```c
    uint64_t historyRequest = 0u;
    struct SubscriberPortData* subscriber = Subscriber_new("Radar", "FrontLeft", "Counter", historyRequest);
    ```
 
  3. We subscribe to the service with a sample cache capacity of 10.
     ```c
     Subscriber_subscribe(subscriber, 10);
     ```

  4. In this loop we receive samples as long the `killswitch` is not
     set to `true` by an external signal and then print the counter 
     value to the console.
     ```c
     while (!killswitch)
     {
         if (SubscribeState_SUBSCRIBED == Subscriber_getSubscriptionState(subscriber))
         {
             const void* chunk = NULL;
             while (ChunkReceiveError_SUCCESS == Subscriber_getChunk(subscriber, &chunk))
             {
                 const struct CounterTopic* sample = (const struct CounterTopic*)(chunk);
                 printf("Receiving: %u\n", sample->counter);
                 Subscriber_releaseChunk(subscriber, chunk);
             }
         }
         else
         {
             printf("Not subscribed!\n");
         }

         sleepFor(1000);
     }
     ```
  
  5. After we stop receiving samples we would like to unsubscribe.
     ```c
     Subscriber_unsubscribe(subscriber);
     ```

  6. When using the C API we have to cleanup the subscriber after 
     its usage.
     ```c
     Subscriber_delete(subscriber);
     ```

### Publisher
The publisher is implemented in a way like in the
[icedelivery C++ example](../icedelivery/).

 1. Create runtime instance.
 2. Create publisher port.
 3. Offer the service 
 4. Send data 
 5. Stop offering the service
 6. **C API: Additionally, we have to remove the Publisher port!**

Let's take a look at the `sending` function which comes with the
`ice_c_publisher.c` example.

 1. We register our process at roudi with the name `iox-c-subscriber`
    ```c
    PoshRuntime_getInstance("/iox-c-publisher");
    ```
 2. We create a publisher with the service 
    {"Radar", "FrontLeft", "Counter"}
    ```c
    uint64_t historyRequest = 0u;
    struct PublisherPortData* publisher = Publisher_new("Radar", "FrontLeft", "Counter", historyRequest);
    ```
 3. We offer our service to the world.
    ```c
    Publisher_offer(publisher);
    ```

 4. Till an external signal sets `killswitch` to `true` we will send an
    incrementing number to all subscribers every send and print the
    value of this number to the console.
    ```c
    uint32_t ct = 0u;
    
    while (!killswitch)
    {
        void* chunk = NULL;
        if (AllocationError_SUCCESS == Publisher_allocateChunk(publisher, &chunk, sizeof(struct CounterTopic)))
        {
            struct CounterTopic* sample = (struct CounterTopic*)chunk;
    
            sample->counter = ct;
    
            printf("Sending: %u\n", ct);
    
            Publisher_sendChunk(publisher, chunk);
    
            ++ct;
    
            sleepFor(1000);
        }
        else
        {
            printf("Failed to allocate chunk!");
        }
    }
    ```

 5. We stop offering our service.
    ```c
    Publisher_stopOffer(publisher);
    ```

 6. And we cleanup our publisher port.
    ```c
    Publisher_delete(publisher);
    ```