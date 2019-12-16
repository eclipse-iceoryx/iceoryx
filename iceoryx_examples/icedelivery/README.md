# icedelivery - Transfer data between POSIX applications

## Introduction

This example showcases a one-way data transmission setup with zero-copy inter-process communication (IPC) on iceoryx.
It provides publisher and subscriber applications. They come in two API flavours (Bare-metal and simplified).

### RouDi, the daemon

RouDi is an abbrevation for **Rou**ting and **Di**scovery. This perfectly describes RouDi's tasks. He takes care of the
communication setup but does not actually participate in the communication between the publisher and the subscriber. Think
of RouDi as the switchboard operator of iceoryx. One of his other major tasks is the setup of the shared memory,
which the applications are using to talk to each other. We currently use memory pools with different chunk sizes,
called in literature a segregated free-list approach. RouDi is delivered pre-built in the Debian package
with a default memory config. We don't support the memory pool configuration with a config file yet, so it has to be
changed in the source file [mepoo_config.cpp](../../iceoryx_posh/source/mepoo/mepoo_config.cpp).
To view the available command line options call `RouDi --help`.

## Run icedelivery

Create three terminals and run one command in each of them. Either choose to run the normal or simplified version.

    # If installed and available in PATH environment variable
    RouDi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/posh/RouDi


    ./build/iceoryx_examples/icedelivery/ice-publisher-bare-metal
    # The simplified publisher is an alternative
    ./build/iceoryx_examples/icedelivery/ice-publisher-simple


    ./build/iceoryx_examples/icedelivery/ice-subscriber-bare-metal
    # The simplified subscriber is an alternative
    ./build/iceoryx_examples/icedelivery/ice-subscriber-simple

## Expected output

The counter can differ depending on startup of the applications.

### RouDi application

    Reserving 99683360 bytes in the shared memory [/iceoryx_mgmt]
    [ Reserving shared memory successful ]
    Reserving 410709312 bytes in the shared memory [/username]
    [ Reserving shared memory successful ]

### Publisher application

    Sending: 0
    Sending: 1
    Sending: 2
    Sending: 3
    Sending: 4
    Sending: 5

### Subscriber application (bare-metal)

    Not subscribed
    Receiving: 3
    Receiving: 4
    Receiving: 5

### Subscriber application (simple)

    Callback: 4
    Callback: 5
    Callback: 6
    Callback: 7

## Code walkthrough

This example makes use of two kind of API flavours. With the bare-metal API you have the most flexibility. It enables us
to put higher level APIs with different look and feel on top of iceoryx. E.g. the ara::com API of AUTOSAR Adaptive or the ROS2 API.
It is not meant to be used by developers in daily life, we assume there will always be a higher abstraction. A simple example
how such an abstraction could look like is given in the second step with the simplified example.

### Publisher application (bare-metal)

First off let's include the publisher and the runtime:

    #include "iceoryx_posh/popo/publisher.hpp"
    #include "iceoryx_posh/runtime/posh_runtime.hpp"

You might be wondering what the publisher application is sending? It's this struct.

    struct CounterTopic
    {
        uint32_t counter;
    };

It is included by:

    #include "topic_data.hpp"

For the communication with RouDi a runtime object is created. The parameter of the method `getInstance()` contains a
unique string identifier for this publisher.

    iox::runtime::PoshRuntime::getInstance("/publisher-bare-metal");

Now that RouDi knows our publisher application is exisiting, let's create a publisher instance and offer our charming struct
to everyone:

    iox::popo::Publisher myPublisher({"Radar", "FrontLeft", "Counter"});
    myPublisher.offer();

The strings inside the first parameter of the constructor of `iox::popo::Publisher` are of the type
`capro::ServiceDescription`. `capro` stands for **ca**nionical **pro**tocol and is used to abstract different
SoA protocols. `Radar` is the service name, `FrontLeft` an instance of the service `Radar` and the third string the
specific event `Counter` of the instance. This service model comes from AUTOSAR. It is maybe not the best fit for
typical publish/subscribe APIs but it allows us a matching to different technologies. The event can be compared to
a topic in other publish/subscribe approaches. The service is not a single request/response thing but an element
for grouping of events and/or methods that can be discovered as a service. Service and instance are like classes and
objects in C++. So you always have a specific instance of a service during runtime. In iceoryx a publisher and
a subscriber only match if all the three IDs match.

Now comes the work mode. Data needs to be created. But hang on.. we need memory first! Let's reserve a chunk of
shared memory:

    auto sample = static_cast<CounterTopic*>(myPublisher.allocateChunk(sizeof(CounterTopic)));

Yep, it's bare-metal! `allocateChunk()` returns a `void*` , that needs to be casted to `CounterTopic`.
Then we can assign the value of `ct` to our counter in the shared memory and send the chunk out to all the subscribers.

    sample->counter = ct;
    myPublisher.sendChunk(sample);

The incrementation and sending of the data is done in a loop every second till the user pressed `Ctrl-C`. It is
captured with the signal handler and stops the loop. At the very end

    myPublisher.stopOffer();

is called to say goodbye to all subscribers who have subscribed so far.

### Subscriber application (bare-metal)

How can the subscriber application get the data the publisher application just transmitted?

Similar to the publisher we need to include the runtime and the subscriber as well as the topic data header:

    #include "iceoryx_posh/popo/subscriber.hpp"
    #include "iceoryx_posh/runtime/posh_runtime.hpp"
    #include "topic_data.hpp"

To make RouDi aware of the subscriber an runtime object is created, once again with a unique identifier string:

    iox::runtime::PoshRuntime::getInstance("/subscriber-bare-metal");

In the next step a subscriber object is created, matching exactly the `capro::ServiceDescription` that the publisher
offered:

    iox::popo::Subscriber mySubscriber({"Radar", "FrontLeft", "Counter"});

After the creation the subscriber object subscribes to the offered data. The cache size is given as a parameter.
Cache size in this case means how many samples the FiFo can hold that is present in the subscriber object.
If the FiFo has an overflow, we release the oldest sample and store the newest one.

    mySubscriber.subscribe(10);

Again in a while-loop we do the following: First check whether our subscriber object has already been subscribed:

    if (iox::popo::SubscriptionState::SUBSCRIBED == mySubscriber.getSubscriptionState())
    {

Let's jump to the else-case beforehand. In case the subscriber is not subscribed, this information is printed to the
terminal:

    else
    {
        std::cout << "Not subscribed" << std::endl;
    }

In case the subscriber is subscribed a local variable that stores a `void*` is created:

    const void * chunk = nullptr;

A nested while-loop is used to pop up to the chunks from the internal FiFo.

    while (mySubscriber.getChunk(&chunk))
    {
        // we know what we expect for the CaPro ID we provided with the subscriber c'tor. So we do a cast here
        auto sample = static_cast<const CounterTopic*>(chunk);

        std::cout << "Receiving: " << sample->counter << std::endl;

        // signal the middleware that this chunk was processed and in no more accesssed by the user side
        mySubscriber.releaseChunk(chunk);
    }

After popping the chunks from the internal FiFo the subscriber application sleeps for a second.

Once the signal handler receives a `Ctrl-C` the outer while loop is exited and the subscriber object is disconnected
by:

    mySubscriber.unsubscribe();

### Publisher application (simple)

The simplified publisher application is an example for a high-level user API and does the same thing as the publisher
described before. In this summary just the differences to the prior publisher application are described.

Starting again with the includes, there is now an additional one:

    #include "a_typed_api.hpp"

The classes `TypedPublisher` and `TypedSubscriber` are defined in this file. In this section we'll take look at the `TypedPublisher`.

The methods `offer()` and `stopOffer()` are called [RAII](https://en.cppreference.com/w/cpp/language/raii)-style in the
constructor and destructor respective.

    TypedPublisher(const iox::capro::ServiceDescription& id)
        : m_publisher(id)
    {
        m_publisher.offer();
    }

    ~TypedPublisher()
    {
        m_publisher.stopOffer();
    }

Instead of instantiating an `iox::popo::Publisher` an object of the `TypedPublisher` is created on the stack:

    TypedPublisher<CounterTopic> myTypedPublisher({"Radar", "FrontRight", "Counter"});

The trasmitted struct `CounterTopic` has to be given as a template parameter.

Another difference to the prior publisher application is the simpler `allocate()` call with the casting wrapped inside
`TypedPublisher`. Reserving shared memory becomes much simplier:

    // allocate a sample
    auto sample = myTypedPublisher.allocate();
    // write the data
    sample->counter = ct;

    std::cout<< "Sending: " << ct << std::endl;

    // pass the ownership to the middleware for sending the sample
    myTypedPublisher.publish(std::move(sample));

Now `allocate()` returns a `std::unique_ptr<TopicType, SampleDeleter<TopicType>>` instead of a `void*` , which
automatically frees the memory when going out of scope. For sening the sample the ownership must be transferred
to the middleware with a move operation.

### Subscriber application (simple)

As with the simplified publisher application there is an additional include:

    #include "a_typed_api.hpp"

An instance of `TypedSubscriber` is created:

    TypedSubscriber<CounterTopic> myTypedSubscriber({"Radar", "FrontRight", "Counter"}, myCallback);

Additional to the `iox::capro::ServiceDescription` the second parameter is a function pointer to a callback function
called when receiving data.

In this case the received data is printed in the callback:

    // the callback for processing the samples
    void myCallback(const CounterTopic& sample)
    {
        std::cout << "Callback: " << sample.counter << std::endl;
    }

The constructor and destructor again automatically handle `subscribe()` and `unsubscribe()`:

    TypedSubscriber(const iox::capro::ServiceDescription& id, OnReceiveCallback<TopicType> callback)
        : m_subscriber(id)
        , m_callback(callback)
    {
        m_subscriber.setReceiveHandler(std::bind(&TypedSubscriber::receiveHandler, this));
        m_subscriber.subscribe();
    }

    ~TypedSubscriber()
    {
        m_subscriber.unsubscribe();
        m_subscriber.unsetReceiveHandler();
    }
