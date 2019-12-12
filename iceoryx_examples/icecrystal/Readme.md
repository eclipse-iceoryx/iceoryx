# icecrystal - Learn how to use the introspection for debugging

## Introduction

This example teaches you how to make use of the introspection for debugging purposes. With the introspection you can
have look into the machine room of iceoryx. It shows live information about the memory usage and all registered
processes. Additionally it shows the sender and receiver ports that are created inside the shared memory.

## Run icecrystal

Create three terminals and run one command in each of them. Either choose to run the normal or simplified version.

    # If installed and available in PATH environment variable
    RouDi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/bin/RouDi

    ./build/icedelivery/ice-publisher-simple

    ./build/icedelivery/ice-subscriber-simple

    ./build/iceoryx_introspection/iceoryx_introspection --all

## Expected output

The counter can differ depending on startup of the applications.

### RouDi appliction

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

    Callback: 4
    Callback: 5
    Callback: 6
    Callback: 7

### Iceoryx introspection

<!-- Add introspection screenshot here -->

## Feature walkthrough

The introspection can be started with several command line arguments:

