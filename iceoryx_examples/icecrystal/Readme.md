# icecrystal - Learn how to use the introspection for debugging

## Introduction

This example teaches you how to make use of the introspection for debugging purposes. With the introspection you can
have look into the machine room of RouDi. The introspection shows live information about the memory usage and all
registered processes. Additionally it shows the sender and receiver ports that are created inside the shared memory.

## Run icecrystal

We reuse the binaries from [icedelivery](../icedelivery/). Create four terminals and run one command in each of them.

    # If installed and available in PATH environment variable
    RouDi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/posh/RouDi

    ./build/iceoryx_examples/icedelivery/ice-publisher-bare-metal

    ./build/iceoryx_examples/icedelivery/ice-subscriber-bare-metal

    ./build/iceoryx_introspection/iceoryx_introspection_client --all

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

### Subscriber application

    Receiving: 4
    Receiving: 5
    Receiving: 6
    Receiving: 7

### Iceoryx introspection application

![introspection_screenshot](https://user-images.githubusercontent.com/8661268/70729509-a515d400-1d03-11ea-877d-69d29efe58c0.png)

## Feature walkthrough

This example does not contain any additional code. The code of the `iceoryx_introspection_client` can be found under
[tools/introspection/](../../tools/introspection/).

The introspection can be started with several command line arguments.

    --mempool         Subscribe to mempool introspection data.

The memory pool view will show all available shared memory segments and its owner. Additionally the total and currently
used chunks are visible as well as the minimal value of free chunks. This can be handy for stress tests to find out if
your memory configuration is valid.

    --process         Subscribe to process introspection data.

The process view will show you the processes, which are currently registered with RouDi and its PID.

    --port            Subscribe to port introspection data.

The port view shows both sender and receiver ports that are created by RouDi in the shared memory. Their respective
service description (service, instance, event) is shown to identify them uniquely. The columns `Process` and
`used by process` display to which process the ports belongs and how they are currently connected. Size in bytes of
both sample size and chunk size (sample size + meta data) and statistical data of `Chunks [/Minute]` is provided as
well. When a sender port instantly provides data to a subscriber with the `subscribe()` call, the `Field` column is
ticked. The service discovery protocol allows one to define the `Propagation scope` of the data. This makes it possible
to forward data onwards to other machines e.g. over network or just consume them internally. When a `Callback` is
registered on subscriber side, the box is ticked accordingly. `FiFo size / capacity` shows the consumption of chunks
on the subscriber side and is a useful column to debug potential memleaks.

    --all             Subscribe to all available introspection data.

`--all` will enable all three views at once.

    -v, --version     Display latest official iceoryx release version and exit.

Make sure that the version number of the introspection exactly matches the version number of RouDi. Currently
we don't guarantee binary compatibility between different versions. With different version numbers things might break.
