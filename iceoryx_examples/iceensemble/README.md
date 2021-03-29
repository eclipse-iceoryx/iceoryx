# iceensemble

## Introduction

A common use case is that often we have multiple sensors sending data of the same type, e.g. LIDAR data, and a subscriber is interested in the data of all those sensors.

This example demonstrates

1. how you can run multiple publisher applications publishing on the same topic
2. how to communicate between C and C++

!!! warning
    If you have built iceoryx with the CMake flag `ONE_TO_MANY_ONLY` publishing with more than one publishers
    on one topic is not supported

## Run iceensemble

The easiest way is the build all examples via `./tools/iceoryx_build_test.sh`. Then, create eight terminals and run one command in each of them.

```sh
# If installed and available in PATH environment variable
iox-roudi
# If build from scratch with script in tools
$ICEORYX_ROOT/build/iox-roudi

build/iceoryx_examples/icedelivery/iox-ex-publisher-helloworld
build/iceoryx_examples/icedelivery/iox-ex-publisher
build/iceoryx_examples/icedelivery/iox-ex-publisher-untyped
build/iceoryx_examples/iceoptions/iox-ex-publisher-with-options
build/iceoryx_examples/icedelivery_in_c/iox-c-publisher

build/iceoryx_examples/icedelivery/iox-ex-subscriber
build/iceoryx_examples/iceoptions/iox-subscriber-with-options
```

Alternatively you can use the provided [tmux](https://en.wikipedia.org/wiki/Tmux) script.

```sh
./run_iceensemble.sh
```

## Expected output

<!-- add asciiema link here -->
