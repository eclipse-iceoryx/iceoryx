# iceperf - Benchmark for the iceoryx transmission latency

## Introduction

This example measures the latency of an IPC transmission between two applications.

## Run iceperf

Create three terminals and run one command in each of them.

    # If installed and available in PATH environment variable
    RouDi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/install/prefix/bin/RouDi


    build/iceoryx_examples/iceperf/iceperf-laurel


    build/iceoryx_examples/iceperf/iceperf-hardy

## Expected output

The counter can differ depending on startup of the applications and the performance of the hardware.

### RouDi application

    Reserving 99683360 bytes in the shared memory [/iceoryx_mgmt]
    [ Reserving shared memory successful ]
    Reserving 410709312 bytes in the shared memory [/username]
    [ Reserving shared memory successful ]

### iceperf-laurel application

    Waiting to subscribe to Hardy ... done
    Waiting for subscriber to Laurel ... done
    Measurement for 1 kB payload ... done
    Measurement for 2 kB payload ... done
    Measurement for 4 kB payload ... done
    Measurement for 8 kB payload ... done
    Measurement for 16 kB payload ... done
    Measurement for 32 kB payload ... done
    Measurement for 64 kB payload ... done
    Measurement for 128 kB payload ... done
    Measurement for 256 kB payload ... done
    Measurement for 512 kB payload ... done
    Measurement for 1024 kB payload ... done
    Measurement for 2048 kB payload ... done
    Measurement for 4096 kB payload ... done
    Waiting for subscriber to unsubscribe from Laurel ... done

    #### Measurement Result ####
    1000000 round trips for each payload.

    | Payload Size [kB] | Average Latency [µs] |
    |------------------:|---------------------:|
    |                 1 |                 0.34 |
    |                 2 |                 0.34 |
    |                 4 |                 0.34 |
    |                 8 |                 0.34 |
    |                16 |                 0.34 |
    |                32 |                 0.34 |
    |                64 |                 0.34 |
    |               128 |                 0.34 |
    |               256 |                 0.34 |
    |               512 |                 0.34 |
    |              1024 |                 0.34 |
    |              2048 |                 0.34 |
    |              4096 |                 0.34 |

    Finished!


### iceperf-hardy application

    Waiting to subscribe to Laurel ... done
    Waiting for subscriber to Hardy ... done
    Waiting for subscriber to unsubscribe from Hardy ... done
    Finished!

## Code walkthrough

This examples measures the latency of an IPC transmission between two applications with several payload sizes.
The measured time is just allocating/releasing chunks and the time to send the chunk.
The construction of the payload is not part of the measurement.

The iceperf-laurel application is allocating chunks with several payload sizes. For each payload size 1000000 round-trips
with iceperf-hardy are performed, which results in 2000000 chunk allocations, transmissions and chunk releases per measurement.

At the end of the benchmark, the average latency for each payload size is printed.

### iceperf-laurel application

First off let's include the publisher, subscriber, runtime and topic data:

```cpp
    #include "iceoryx_posh/popo/publisher.hpp"
    #include "iceoryx_posh/popo/subscriber.hpp"
    #include "iceoryx_posh/runtime/posh_runtime.hpp"
    #include "topic_data.hpp"
```

Independent of the real payload size, this struct is used to transfer some information between the applications.

```cpp
    struct PerfTopic
    {
        uint64_t payloadSize {0};
        bool run {true};
    };
```

With `payloadSize` as the payload size used for the current measurement and `run` to shutdown iceperf-hardy at the end of the benchmark.

Let's set some constants to prevent magic values.
```cpp
    constexpr uint64_t NUMBER_OF_ROUNDTRIPS{1000000};
    constexpr char APP_NAME[] = "/laurel";
    constexpr char PUBLISHER[] = "Laurel";
    constexpr char SUBSCRIBER[] = "Hardy";
```

For the communication with RouDi a runtime object is created. The parameter of the method `getInstance()` contains a
unique string identifier for this publisher.

```cpp
    iox::runtime::PoshRuntime::getInstance(APP_NAME);
```

Now that RouDi knows our applications exist, let's create the publisher and subscriber instances
and offer the iceperf-larry service and subscribe to iceperf-hardy service:

```cpp
    iox::popo::Publisher myPublisher({"Comedians", "Duo", PUBLISHER});
    myPublisher.offer();

    iox::popo::Subscriber mySubscriber({"Comedians", "Duo", SUBSCRIBER});
    mySubscriber.subscribe(1);
```

For the benchmark it's important that `Laurel` is subscribed to `Hardy` and vice versa.

```cpp
    while (mySubscriber.getSubscriptionState() != iox::popo::SubscriptionState::SUBSCRIBED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    while (!myPublisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
```

The following payload sizes are used. The size is specified in kB.


```cpp
const std::vector<int64_t> payloadSizesInKB{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
```

For each payload, an initial sample is send. This specifies the payload for the measurement,
since in `measureLatency(...)` this data is used to allocate the chunk for the response.
The publisher doesn't use a constant payload size, but a dynamic one. This has to be enabled
by setting the second parameter of `allocateChunk(...)` to `true`. If not set to `true`,
a changed payload, while being subscribed, is assumed to be an error.

```cpp
    for (const auto payloadSizeInKB : payloadSizesInKB)
    {
        auto payloadSizeInBytes = payloadSizeInKB * 1024;
        auto sample = static_cast<PerfTopic*>(myPublisher.allocateChunk(payloadSizeInBytes, true));

        // Specify the payload size for the measurement
        sample->payloadSize = payloadSizeInBytes;
        sample->run = true;

        // Send the initial sample to start the round-trips
        myPublisher.sendChunk(sample);

        // perform the actual measurement
        auto latency = measureLatency(myPublisher, mySubscriber);
        latencyInMicroSeconds.push_back(latency);

        // Wait for hardy to send the last response
        const void* receivedChunk;
        while (!mySubscriber.getChunk(&receivedChunk))
        {
            // poll as fast as possible
        }
        mySubscriber.releaseChunk(receivedChunk);
    }
```

The actual measurement is performed in the `measureLatency(...)` function. For each round-trip,
at first the subscriber is polling for the chunk, then it allocates a new chunk with the payload
specified in the received `PerfTopic` chunk, responds with that chunk and releases the old chunk.
The time for all round-trips is measured and the average latency is returned.

```cpp
    double measureLatency(iox::popo::Publisher& publisher, iox::popo::Subscriber& subscriber)
    {
        auto start = std::chrono::high_resolution_clock::now();
        // run the performance test
        for (uint64_t i = 0; i < NUMBER_OF_ROUNDTRIPS; ++i)
        {
            const void* receivedChunk;
            while (!subscriber.getChunk(&receivedChunk))
            {
                // poll as fast as possible
            }

            auto receivedSample = static_cast<const PerfTopic*>(receivedChunk);

            auto sendSample = static_cast<PerfTopic*>(publisher.allocateChunk(receivedSample->payloadSize, true));
            sendSample->payloadSize = receivedSample->payloadSize;
            sendSample->run = true;

            publisher.sendChunk(sendSample);

            subscriber.releaseChunk(receivedChunk);
        }

        auto finish = std::chrono::high_resolution_clock::now();

        constexpr uint64_t TRANSMISSIONS_PER_ROUNDTRIP{2};
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
        auto latencyInNanoSeconds = (duration.count() / (NUMBER_OF_ROUNDTRIPS * TRANSMISSIONS_PER_ROUNDTRIP));
        auto latencyInMicroSeconds = latencyInNanoSeconds / 1000.;
        return latencyInMicroSeconds;
    }
```

After the benchmark, `Hardy` needs to be shut down. This is done by setting the `run` flag to `false` on the last message to `Hardy`.

```cpp
    const int64_t payloadSize = sizeof(PerfTopic);
    auto stopSample = static_cast<PerfTopic*>(myPublisher.allocateChunk(payloadSize, true));

    stopSample->payloadSize = payloadSize;
    stopSample->run = false;
    myPublisher.sendChunk(stopSample);
```

Then we unsubscribe, wait till `Laurel` has no subscribers and stop offering our service.

```cpp
    mySubscriber.unsubscribe();

    while (!myPublisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    myPublisher.stopOffer();
```

At the end, the result of the benchmark is printed.

```cpp
    for (size_t i = 0; i < latencyInMicroSeconds.size(); ++i)
    {
        std::cout << "| " << std::setw(17) << payloadSizesInKB.at(i) << " | " << std::setw(20) << std::setprecision(2)
                  << latencyInMicroSeconds.at(i) << " |" << std::endl;
    }
```

### iceperf-hardy application

The boilerplate code (getting runtime, publish and subscribe) for iceperf-hardy is similar to iceperf-laurel.

The main difference is in the main loop, where `Hardy` is just waiting for data from `Laurel`, immediately responses
and shuts down when requested.

```cpp
    bool run{true};
    while (run)
    {
        const void* receivedChunk;
        while (!mySubscriber.getChunk(&receivedChunk))
        {
            // poll as fast as possible
        }

        auto receivedSample = static_cast<const PerfTopic*>(receivedChunk);

        auto sendSample = static_cast<PerfTopic*>(myPublisher.allocateChunk(receivedSample->payloadSize, true));
        sendSample->payloadSize = receivedSample->payloadSize;

        myPublisher.sendChunk(sendSample);

        run = receivedSample->run;
        mySubscriber.releaseChunk(receivedChunk);
    }
```
