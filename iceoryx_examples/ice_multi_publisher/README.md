# Multiple publishers for the same topic

## Introduction
A common use case is that often we have multiple sensors sending data of the same type, e.g. LIDAR data, and a subscriber is interested in the data of all those sensors. 

This example shows how you can run multiple publishers and let them publish on the same topic. 
Here we only cover the specifics for this use case, for basic sending and receiving see the 
 [icedelivery](../icedelivery/README.md) example. The example also demonstrates how you can change the maximum number of samples that can be queued up at a subscriber when you resubscribe.
## Run ice_multi_publisher

Create four terminals and run one command in each of them. 

    # If installed and available in PATH environment variable
    iox-roudi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/posh/iox-roudi

    ./build/iceoryx_examples/ice_multi_publisher/iox-multi-publisher

    ./build/iceoryx_examples/ice_multi_publisher/iox-subscriber

    ./build/iceoryx_examples/ice_multi_publisher/iox-resubscriber


This starts two publishers in the iox-multi-publisher application and 
a subscriber in each of the other applications. The two publishers publish two independent counters. The data of both is made distinct by using an id, which has the values 1 and 2 respectively. 
## Expected output

The counters can differ depending on startup of the applications.

### RouDi application

    Reserving 84783040 bytes in the shared memory [/iceoryx_mgmt]
    [ Reserving shared memory successful ] 
    Reserving 149655680 bytes in the shared memory [/apex]
    [ Reserving shared memory successful ] 
    RouDi is ready for clients

### Publisher application

    Counter Instance sending: id 1 counter 0
    Counter Instance sending: id 2 counter 0
    Counter Instance sending: id 1 counter 1
    Counter Instance sending: id 2 counter 1
    Counter Instance sending: id 1 counter 2
    Counter Instance sending: id 1 counter 3
    Counter Instance sending: id 2 counter 2
    Counter Instance sending: id 1 counter 4
    Counter Instance sending: id 1 counter 5
    Counter Instance sending: id 2 counter 3
    Counter Instance sending: id 1 counter 6
### Subscriber application

    Waiting for data ... 
    Received: id 1 counter 1
    Received: id 2 counter 1
    Received: id 1 counter 2
    Waiting for data ... 
    Received: id 1 counter 3
    Received: id 2 counter 2
    Received: id 1 counter 4
    Waiting for data ... 
    Received: id 1 counter 5
    Received: id 2 counter 3
    Received: id 1 counter 6

The subscriber application wakes up periodically, looks for data, and if it received any displays the data. It can be seen that the data arrives in order for any publisher, indicated by a monotonic counter per id. The order of data of different id is indeterminate since the publishers are sending concurrently.

### Resubscriber application

    Waiting for data ... 
    Unsubscribed ... Subscribe in 3 seconds
    Subscribe with max number of samples 4
    Received: id 1 counter 23
    Received: id 2 counter 12
    Received: id 1 counter 24
    Waiting for data ... 
    Unsubscribed ... Subscribe in 3 seconds
    Subscribe with max number of samples 1
    Received: id 1 counter 32
    Waiting for data ... 
    Unsubscribed ... Subscribe in 3 seconds
    Subscribe with max number of samples 2
    Received: id 2 counter 20
    Received: id 1 counter 40
    Waiting for data ... 

The resubscriber application periodically unsubscribes and then subscribes again with a different number of samples which the subscriber can queue up. It can be seen that no more than this number of samples are received before unsubscribing again (due to the limited queue size specified upon re-subscription). During the time in which it is unsubscribed data is lost, but when it resubscribes there may already be data available which is then displayed. Due to asynchronous sending and subscribing, it cannot be guaranteed that data is available immediately on re-subscribtion and the data can be new (after resubscribe) or stale (before unsubscribe).  


## Code walkthrough

We focus on the aspects in which this example extends the icedelivery example and uses features not showcased there. More details on how to setup the publish subscribe communication can be found in [icedelivery](../icedelivery/README.md). 

### The topic

The topic is a basic struct which consists of a counter and an id. In principle it could be any class which satisfies some mild conditions, such as being default constructible and copyable.

The id is used to distinguish different publishers.

    struct CounterTopic
    {
        uint32_t counter;
        uint32_t id;
    };

We also provide ``operator<<`` to be able to display the data.

    std::ostream& operator<<(std::ostream& s, const CounterTopic& topic)
    {
        s << "id " << topic.id << " counter " << topic.counter;
        return s;
    }

### Multi-Publisher application

We create a publisher with the following calls. The three string arguments allow a fine-grained control
to specify matching topics. The topic has a data type ``CounterTopic`` and a topic name 
``Counter``. In addition it belongs to some ``Group`` and is a specific ``instance``. For subscription purposes all three identifiers must match on subscriber side. It is possible to only specify the topic name and set the others to some default string for all topics.

If some identifier is only known at runtime (e.g. it is read from some config file), you have to create an ``IdString`` first before passing it to the ``TypedPublisher`` constructor. This is done here for ``instance``, which is created from some ``instanceName``. 


    iox::capro::IdString instance{iox::cxx::TruncateToCapacity, instanceName};
    iox::popo::TypedPublisher<CounterTopic> publisher({"Group", instance, "Counter"});


After construction, we immediately offer the topic and start sending data.

    publisher.offer();

    for (uint32_t counter = 0U; !killswitch; ++counter)
    {
        CounterTopic data{counter, id};
        publisher.publishCopyOf(data);
        //...
    }

The data consists of an id (chosen to be different for each publisher to distinguish their data) and a monotonically increasing counter. This counter is send periodically and we leave the loop when Ctrl-C is pressed and stop offering.

    publisher.stopOffer();


In the main function two threads are started, each of them corresponding to a publisher.
Notice that they have different ids but use the same ``"Instance"``. While the instance could have been a string literal in the publisher constructor, this way it is possible to choose the instance name at runtime. This could be done for the group and topic identifier as well. Finally both publishers send at different time intervals, roughly 500ms and 1000ms.

    std::thread sender1(send, 1, "Instance", std::chrono::milliseconds(500));
    std::thread sender2(send, 2, "Instance", std::chrono::milliseconds(1000));

Once Ctrl+C is pressed, we leave the publisher loops and join the threads.

    sender1.join();
    sender2.join();

### Subscriber application

We create a subscriber via
    iox::popo::TypedSubscriber<CounterTopic> subscriber({"Group", "Instance", "Counter"});

and immediately subscribe.
    subscriber.subscribe();

Notice that all identifiers match the ones provided by the two publishers.

We periodically wake up

    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (subscriber.hasNewSamples())

When there are new samples we display them on the console.

    subscriber.take()
        .and_then([](iox::popo::Sample<const CounterTopic>& sample) {
            std::cout << "Received: " << *sample.get() << std::endl;
        })

The topic is displayed by providing a defintion of ``operator<<`` which prints the id and counter to the console.
The displayed counters are monotonically increasing for each id but between different publishers the order of data arrival is indeterminate due to concurrent sending.

We also handle potential errors

    .or_else([](iox::popo::ChunkReceiveError) { std::cout << "Error while receiving." << std::endl; });

and wait for some time before looking for data again.

    std::cout << "Waiting for data ... " << std::endl;

When Ctrl+C is pressed we exit the loop and unsubscribe

    subscriber.unsubscribe();

before joining the receiver thread

    receiver.join();

### Resubscriber application

We first define a bound for the maximum number of samples the subscriber can queue up.

    constexpr uint64_t MAX_NUMBER_OF_SAMPLES{4U};

We also define some time period for which we will later unsubscribe.

    constexpr uint64_t UNSUBSCRIBED_TIME_SECONDS{3U};

We create a subscriber via
    iox::popo::TypedSubscriber<CounterTopic> subscriber({"Group", "Instance", "Counter"});

and immediately subscribe.
    subscriber.subscribe();

The topic we subscribe to is the same as for the subscriber application.

We periodically unsubscribe for a specified time.

    subscriber.unsubscribe();
    std::cout << "Unsubscribed ... Subscribe in " << UNSUBSCRIBED_TIME_SECONDS << " seconds" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(UNSUBSCRIBED_TIME_SECONDS));

We resubscribe and specify the maximum number of samples the subscriber is able to queue up ``maxNumSamples``. 

    maxNumSamples = maxNumSamples % MAX_NUMBER_OF_SAMPLES + 1U; 
    subscriber.subscribe(maxNumSamples);

This number changes cyclically from 1 up to 4. This means that in some cases we will only receive one sample, even when more were send by the two publishers.

We wait for some time, check for data, and if there is any display it on the console.

    std::this_thread::sleep_for(std::chrono::seconds(1));

        while (subscriber.hasNewSamples())
        {
            subscriber.take()
                .and_then([](iox::popo::Sample<const CounterTopic>& sample) {
                    std::cout << "Received: " << *sample.get() << std::endl;
                })
                .or_else([](iox::popo::ChunkReceiveError) { std::cout << "Error while receiving." << std::endl; });
        };
        std::cout << "Waiting for data ... " << std::endl;

Depending on the ``maxNumSamples`` we specified on subscription we will see the most recent data from the two publishers, but never more than ``maxNumSamples``. Stale data is silently discarded. This shows that the internal queue size on the subscriber size can be changed during operation. Note that again it is indeterminate which data we see if both publishers are sending concurrently. In particular if the subscriber can only hold one sample (``maxNumSamples=1``), we may see data with id 1 or 2.

When Ctrl+C is pressed we exit the loop and unsubscribe

    subscriber.unsubscribe();

before joining the receiver thread

    receiver.join();













