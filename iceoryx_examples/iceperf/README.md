# iceperf - Benchmark for the iceoryx transmission latency

## Introduction

This example measures the latency of IPC transmissions between two applications.
We compare the latency of iceoryx with message queues and unix domain sockets.

The measurement is carried out with several payload sizes. Round trips are performed
for each payload size, using either the default setting or the provided command line parameter
for the number of round trips to do. 
The measured time is just allocating/releasing memory and the time to send the data.
The construction and writing of the payload is not part of the measurement.

At the end of the benchmark, the average latency for each payload size is printed.

## Run iceperf

Create three terminals and run one command in each of them. 
The order is first the RouDi daemon, then iceperf-laurel which is the leader in this setup
and then iceperf-laurel for doing the ping pong measurements with iceperf-laurel. 
You can set the number of measurement iterations (number of roundtrips) with a command line paramter
of iceperf-laurel (e.g. ./iceperf-laurel 100000)

    # If installed and available in PATH environment variable
    iox-roudi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/install/prefix/bin/iox-roudi

    build/iceoryx_examples/iceperf/iceperf-laurel

    build/iceoryx_examples/iceperf/iceperf-hardy

## Expected output

The numbers will differ depending on parameters and the performance of the hardware.
Which technologies are measured depends on the operating system (e.g. no message queue on MacOS).
Here an example output with Ubuntu 18.04 on Intel(R) Xeon(R) CPU E3-1505M v5 @ 2.80GHz.

### iceperf-laurel application

    ******   MESSAGE QUEUE    ********
    waiting for follower
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

    #### Measurement Result ####
    100000 round trips for each payload.

    | Payload Size [kB] | Average Latency [µs] |
    |------------------:|---------------------:|
    |                 1 |                  3.1 |
    |                 2 |                  3.2 |
    |                 4 |                  3.8 |
    |                 8 |                  5.2 |
    |                16 |                  7.7 |
    |                32 |                   13 |
    |                64 |                   23 |
    |               128 |                   43 |
    |               256 |                   81 |
    |               512 |              1.6e+02 |
    |              1024 |                3e+02 |
    |              2048 |              5.9e+02 |
    |              4096 |              1.2e+03 |

    Finished!

    ****** UNIX DOMAIN SOCKET ********
    waiting for follower
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

    #### Measurement Result ####
    100000 round trips for each payload.

    | Payload Size [kB] | Average Latency [µs] |
    |------------------:|---------------------:|
    |                 1 |                  4.3 |
    |                 2 |                  4.3 |
    |                 4 |                  4.6 |
    |                 8 |                    6 |
    |                16 |                  8.7 |
    |                32 |                   14 |
    |                64 |                   27 |
    |               128 |                   53 |
    |               256 |              1.1e+02 |
    |               512 |              2.1e+02 |
    |              1024 |              4.2e+02 |
    |              2048 |              8.4e+02 |
    |              4096 |              1.7e+03 |

    Finished!

    ******      ICEORYX       ********
    Waiting till subscribed ...
    Waiting for subscriber ...
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
    Waiting for subscriber to unsubscribe ...
    Finished!

    #### Measurement Result ####
    100000 round trips for each payload.

    | Payload Size [kB] | Average Latency [µs] |
    |------------------:|---------------------:|
    |                 1 |                 0.73 |
    |                 2 |                 0.58 |
    |                 4 |                 0.61 |
    |                 8 |                 0.61 |
    |                16 |                 0.59 |
    |                32 |                 0.62 |
    |                64 |                  0.6 |
    |               128 |                 0.58 |
    |               256 |                 0.61 |
    |               512 |                 0.61 |
    |              1024 |                 0.58 |
    |              2048 |                 0.61 |
    |              4096 |                 0.61 |

    Finished!

### iceperf-hardy application

    ******   MESSAGE QUEUE    ********
    registering with the leader, if no leader this will crash with a message queue error now

    ****** UNIX DOMAIN SOCKET ********
    registering with the leader, if no leader this will crash with a socket error now

    ******      ICEORYX       ********
    Waiting till subscribed ... 
    Waiting for subscriber ... 
    Waiting for subscriber to unsubscribe ... 
    Finished!

## Code walkthrough

Here we roughly describe the setup for performing the measurements. Things like initialization, sending and receiving of data are technology specific and can be found in the respective files (e.g. uds.cpp for 
unix domain socket). Our focus here is on the abstraction layer on top which allows us to add new IPC technologies or you to extend and compare with whatever.

### iceperf-laurel application

Besides includes for the different IPC technologies, the topic_data.hpp file is included that contains the PerTopic struct which is used to transfer some information between the applications. Independent of the real payload size, this struct is used as some kind of header in each transferred sample. 

```cpp
    struct PerfTopic
    {
        uint32_t payloadSize{0};
        uint32_t subPackets{0};
        bool run{true};
    };
```

With `payloadSize` as the payload size used for the current measurement. In case it is not possible to transfer the `payloadSize` with a single data transfer (e.g. OS limit for the payload of a single socket send), the payload is divided in several sub-packets. This is indicated with `subPackets`. The `run` flag is used to shutdown iceperf-hardy at the end of the benchmark.

Let's set some constants to prevent magic values. The default number of round trips is set and names for the communication resources that are used. 
```cpp
    constexpr int64_t NUMBER_OF_ROUNDTRIPS{10000};
    constexpr char APP_NAME[] = "/laurel";
    constexpr char PUBLISHER[] = "Laurel";
    constexpr char SUBSCRIBER[] = "Hardy";
```

The `leaderDo()` function executes a measurement for the provided IPC technology and number of round trips. For being able to always perform the same steps and avoiding code duplications, we use a base class with the interface to implement for each technology and the technology independent functionality. 

```cpp
    void leaderDo(IcePerfBase& ipcTechnology, int64_t numRoundtrips)
    {
        ipcTechnology.initLeader();

        std::vector<double> latencyInMicroSeconds;
        const std::vector<uint32_t> payloadSizesInKB{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
        for (const auto payloadSizeInKB : payloadSizesInKB)
        {
            std::cout << "Measurement for " << payloadSizeInKB << " kB payload ... " << std::flush;
            auto payloadSizeInBytes = payloadSizeInKB * IcePerfBase::ONE_KILOBYTE;

            ipcTechnology.prePingPongLeader(payloadSizeInBytes);

            auto latency = ipcTechnology.pingPongLeader(numRoundtrips);

            latencyInMicroSeconds.push_back(latency);

            ipcTechnology.postPingPongLeader();
        }
        ipcTechnology.releaseFollower();

        ipcTechnology.shutdown();

        std::cout << std::endl;
        std::cout << "#### Measurement Result ####" << std::endl;
        std::cout << numRoundtrips << " round trips for each payload." << std::endl;
        std::cout << std::endl;
        std::cout << "| Payload Size [kB] | Average Latency [µs] |" << std::endl;
        std::cout << "|------------------:|---------------------:|" << std::endl;
        for (size_t i = 0; i < latencyInMicroSeconds.size(); ++i)
        {
            std::cout << "| " << std::setw(17) << payloadSizesInKB.at(i) << " | " << std::setw(20) << std::setprecision(2)
                    << latencyInMicroSeconds.at(i) << " |" << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Finished!" << std::endl;
}
```

Initialization is different for each IPC technology. Here we have to create sockets, message queues or iceoryx publisher and subscriber. With `ipcTechnology.initLeader()` we are setting up these resources on the leader side. After the definition of the different payload sizes to use, we execute a single round trip measurement for each individual payload size. The leader has to orchestrate the whole process and has a pre and post step for each ping pong round trip measurement. `ipcTechnology.prePingPongLeader()` sets the payload size for the upcoming measurement. `ipcTechnology.pingPongLeader(numRoundtrips)` then does the ping pong between leader and follower and returns the time it took to do the provided number of round trips. After the measurments were done for all the different payload sizes, `ipcTechnology.releaseFollower()` releases the follower that is not aware of things like how many payload sizes are considered. After cleaning up the communication resources with `ipcTechnology.shutdown()` the results are printed. 

In the `main()` method we create instances for the different IPC technologies we want to compare. Each one is implemented in an own class and implements the pure virtual functions provided with the `IcePerfBase` class

iceperf-laurel, the leader in this setup, takes the number of round trips to perform for each payload size as command line parameter. We check if one was provided and take this or otherwise use the default one. The higher the number of round trips, the more accurate the measurements will be but you have to consider that an iceperf run can take quite a long time then.   

```cpp
        uint64_t numRoundtrips = NUMBER_OF_ROUNDTRIPS;
        if (argc > 1)
        {
            if (!iox::cxx::convert::fromString(argv[1], numRoundtrips))
            {
                std::cout << "error command line parameter" << std::endl;
                exit(1);
            }
        }
```

Now we can create an object for each IPC technology that we want to evaluate and call the `leaderDo()` function. The naming conventions for the different technologies differ, therefore we do some prefixing if necessary

```cpp
    #ifndef __APPLE__
        std::cout << std::endl << "******   MESSAGE QUEUE    ********" << std::endl;
        MQ mq("/" + std::string(PUBLISHER), "/" + std::string(SUBSCRIBER));
        leaderDo(mq, numRoundtrips);
    #endif

        std::cout << std::endl << "****** UNIX DOMAIN SOCKET ********" << std::endl;
        UDS uds("/tmp/" + std::string(PUBLISHER), "/tmp/" + std::string(SUBSCRIBER));
        leaderDo(uds, numRoundtrips);

        std::cout << std::endl << "******      ICEORYX       ********" << std::endl;
        iox::runtime::PoshRuntime::getInstance(APP_NAME); // runtime for registering with the RouDi daemon
        Iceoryx iceoryx(PUBLISHER, SUBSCRIBER);
        leaderDo(iceoryx, numRoundtrips);
```

### iceperf-hardy application

The `main()` for iceperf-hardy is similar to iceperf-laurel, only the SUBSCRIBER and PUBLISHER names changed to the other way round. The `followerDo()` function is much simpler as the follower only reacts and does not the control. Besides `ipcTechnology.initFollower()` and `ipcTechnology.shutdown()` all the functionality to do the ping pong for different payload sizes is done in `ipcTechnology.pingPongFollower()`

```cpp
    void followerDo(IcePerfBase& ipcTechnology)
    {
        ipcTechnology.initFollower();

        ipcTechnology.pingPongFollower();

        ipcTechnology.shutdown();
    }
```
