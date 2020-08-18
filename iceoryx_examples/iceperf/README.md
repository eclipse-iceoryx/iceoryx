# iceperf - Benchmark for the iceoryx transmission latency

## Introduction

This example measures the latency of IPC transmissions between two applications.
We compare the latency of iceoryx with message queues and unix domain sockets.

The measurment is done with several payload sizes. For each payload size the default or
with command line parameter provided number of round-trips are performed.
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

The counter can differ depending on startup of the applications and the performance of the hardware.
Wich technologies are measured depends on the operating system (e.g. no message queue on MacOS)

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
    100 round trips for each payload.

    | Payload Size [kB] | Average Latency [µs] |
    |------------------:|---------------------:|
    |                 1 |                   75 |
    |                 2 |                1e+02 |
    |                 4 |                   98 |
    |                 8 |              1.1e+02 |
    |                16 |                   84 |
    |                32 |              1.9e+02 |
    |                64 |              1.4e+02 |
    |               128 |              2.7e+02 |
    |               256 |              3.7e+02 |
    |               512 |              4.5e+02 |
    |              1024 |              6.9e+02 |
    |              2048 |              1.3e+03 |
    |              4096 |              3.4e+03 |

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
    100 round trips for each payload.

    | Payload Size [kB] | Average Latency [µs] |
    |------------------:|---------------------:|
    |                 1 |                   74 |
    |                 2 |                  8.6 |
    |                 4 |                  8.6 |
    |                 8 |                   56 |
    |                16 |                   16 |
    |                32 |                   30 |
    |                64 |                   73 |
    |               128 |              3.6e+02 |
    |               256 |              3.7e+02 |
    |               512 |              4.8e+02 |
    |              1024 |              8.3e+02 |
    |              2048 |              2.7e+03 |
    |              4096 |              7.3e+03 |

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
    100 round trips for each payload.

    | Payload Size [kB] | Average Latency [µs] |
    |------------------:|---------------------:|
    |                 1 |                   18 |
    |                 2 |                    7 |
    |                 4 |                  5.5 |
    |                 8 |                  5.4 |
    |                16 |                  4.5 |
    |                32 |                  4.5 |
    |                64 |                   29 |
    |               128 |                  5.3 |
    |               256 |                   27 |
    |               512 |                   79 |
    |              1024 |                  6.7 |
    |              2048 |                  8.6 |
    |              4096 |                   62 |

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

