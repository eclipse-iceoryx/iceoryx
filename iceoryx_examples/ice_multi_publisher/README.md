# Multiple publishers for the same topic

## Introduction
A common use case is that often we have multiple sensors sending data of the same type, e.g. LIDAR data, and a subscriber is interested in the data of all those sensors. 

This example shows how you can run multiple publishers and let them publish on the same topic. 
Here we only cover the specifics for this use case, for basic sending and receiving see the 
 [icedelivery](../icedelivery/README.md) example.
## Run ice_multi_publisher

Create four terminals and run one command in each of them. 
```sh
# If installed and available in PATH environment variable
iox-roudi
# If build from scratch with script in tools
$ICEORYX_ROOT/build/posh/iox-roudi

build/iceoryx_examples/ice_multi_publisher/iox-multi-publisher

build/iceoryx_examples/ice_multi_publisher/iox-subscriber
```

This starts two publishers in the iox-multi-publisher application and 
a subscriber in each of the other applications. The two publishers publish two independent counters. The data of both is made distinct by using an id, which has the values 1 and 2 respectively. 
## Expected output

The counters can differ depending on startup of the applications.

### RouDi application
```
2020-12-20 15:55:18.616 [ Info  ]: No config file provided and also not found at '/etc/iceoryx/roudi_config.toml'. Falling back to built-in config.
Log level set to: [Warning]
Reserving 64244064 bytes in the shared memory [/iceoryx_mgmt]
[ Reserving shared memory successful ]
Reserving 149134400 bytes in the shared memory [/user]
[ Reserving shared memory successful ]
RouDi is ready for clients
```

### Publisher application
```
2020-12-20 16:26:42.790 [ Debug ]: Application registered management segment 0x7f378555e000 with size 64244064 to id 1
2020-12-20 16:26:42.790 [ Info  ]: Application registered payload segment 0x7f377c4e6000 with size 149134400 to id 2
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
```

### Subscriber application
```
2020-12-20 16:26:58.839 [ Debug ]: Application registered management segment 0x7f6353c04000 with size 64244064 to id 1
2020-12-20 16:26:58.839 [ Info  ]: Application registered payload segment 0x7f634ab8c000 with size 149134400 to id 2
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
```

The subscriber application wakes up periodically, looks for data, and if it received any displays the data. It can be seen that the data arrives in order for any publisher, indicated by a monotonic counter per id. The order of data of different id is indeterminate since the publishers are sending concurrently.

## Code walkthrough

We focus on the aspects in which this example extends the icedelivery example and uses features not showcased there. More details on how to setup the publish subscribe communication can be found in [icedelivery](../icedelivery/README.md).

### The topic

The topic is a basic struct which consists of a counter and an id. In principle it could be any class which satisfies some mild conditions, such as being default constructible and copyable.

The id is used to distinguish different publishers.
```cpp
struct CounterTopic
{
    uint32_t counter;
    uint32_t id;
};
```

We also provide ``operator<<`` to be able to display the data.
```cpp
std::ostream& operator<<(std::ostream& s, const CounterTopic& topic)
{
    s << "id " << topic.id << " counter " << topic.counter;
    return s;
}
```

### Multi-Publisher application

We create a publisher with the following calls. The three string arguments allow a fine-grained control
to specify matching topics. The topic has a data type ``CounterTopic`` and a topic name 
``Counter``. In addition it belongs to some ``Group`` and is a specific ``instance``. For subscription purposes all three identifiers must match on subscriber side. It is possible to only specify the topic name and set the others to some default string for all topics.

If some identifier is only known at runtime (e.g. it is read from some config file), you have to create an ``IdString`` first before passing it to the ``Publisher`` constructor. This is done here for ``instance``, which is created from some ``instanceName``.

```cpp
iox::capro::IdString instance{iox::cxx::TruncateToCapacity, instanceName};
iox::popo::Publisher<CounterTopic> publisher({"Group", instance, "Counter"});
```

After construction, we immediately start sending data.
```cpp
for (uint32_t counter = 0U; !killswitch; ++counter)
{
    CounterTopic data{counter, id};
    publisher.publishCopyOf(data).or_else([](auto) { std::cerr << "failed to send data" << std::endl; });
    //...
}
```

The data consists of an id (chosen to be different for each publisher to distinguish their data) and a monotonically increasing counter. This counter is send periodically and we leave the loop when Ctrl-C is pressed and stop offering.
```cpp
publisher.stopOffer();
```

In the main function two threads are started, each of them corresponding to a publisher.
Notice that they have different ids but use the same ``"Instance"``. While the instance could have been a string literal in the publisher constructor, this way it is possible to choose the instance name at runtime. This could be done for the group and topic identifier as well. Finally both publishers send at different time intervals, roughly 500ms and 1000ms.
```cpp
std::thread sender1(send, 1, "Instance", std::chrono::milliseconds(500));
std::thread sender2(send, 2, "Instance", std::chrono::milliseconds(1000));
```

Once Ctrl+C is pressed, we leave the publisher loops and join the threads.
```cpp
sender1.join();
sender2.join();
```

### Subscriber application

We create a subscriber via
```cpp
iox::popo::Subscriber<CounterTopic> subscriber({"Group", "Instance", "Counter"});
```

Notice that all identifiers match the ones provided by the two publishers.

We periodically wake up
```cpp
std::this_thread::sleep_for(std::chrono::seconds(1));
while (subscriber.hasData())
```

When there are new samples we display them on the console.
```cpp
subscriber.take()
    .and_then([](iox::popo::Sample<const CounterTopic>& sample) {
        std::cout << "Received: " << *sample.get() << std::endl;
    })
```

The topic is displayed by providing a definition of ``operator<<`` which prints the id and counter to the console.
The displayed counters are monotonically increasing for each id but between different publishers the order of data arrival is indeterminate due to concurrent sending.

We also handle potential errors
```cpp
    .or_else([](iox::popo::ChunkReceiveResult) { std::cout << "Error while receiving." << std::endl; });
```

and wait for some time before looking for data again.
```cpp
std::cout << "Waiting for data ... " << std::endl;
```

When Ctrl+C is pressed we exit the loop

before joining the receiver thread
```cpp
receiver.join();
```
