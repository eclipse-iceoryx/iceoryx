# icecrystal

## Introduction

> This Readme.md is slightly outdated and not all functionality of the introspection is available with v1.0

This example teaches you how to make use of the introspection for debugging purposes. With the introspection you can
look into the machine room of RouDi. The introspection shows live information about the memory usage and all
registered processes. Additionally, it shows the publisher and subscriber ports that are created inside the shared
memory.

## Run icecrystal

We reuse the binaries from [icedelivery](../icedelivery/). Create four terminals and run one command in each of them.
```sh
# If installed and available in PATH environment variable
iox-roudi
# If build from scratch with script in tools
$ICEORYX_ROOT/build/iox-roudi

build/iceoryx_examples/icedelivery/iox-ex-publisher-untyped

build/iceoryx_examples/icedelivery/iox-ex-subscriber-untyped

build/iox-introspection-client --all
```

<!-- @todo Add expected output of RouDi, publisher, subscriber and introspection with asciinema recording before v1.0-->

## Expected output

![introspection_screenshot](https://user-images.githubusercontent.com/22388003/75041206-672feb80-54bc-11ea-8621-2acf95bf376e.png)

## Feature walkthrough

This example does not contain any additional code. The code of the `iceoryx_introspection_client` can be found under
[tools/introspection/](../../tools/introspection/).

The introspection can be started with several command line arguments.

    --mempool         Subscribe to mempool introspection data.

The memory pool view will show all available shared memory segments and their respective owner. Additionally, the
maximum number of available chunks, the number of currently used chunks as well as the minimal value of free chunks
are visible. This can be handy for stress tests to find out if your memory configuration is valid.

    --process         Subscribe to process introspection data.

The process view will show you the processes (incl. PID), which are currently registered with RouDi.

    --port            Subscribe to port introspection data.

The port view shows both publisher and subscriber ports that are created by RouDi in the shared memory. Their respective
service description (service, instance, event) is shown to identify them uniquely. The columns `Process` and
`used by process` display to which process the ports belong and how they are currently connected. Size in bytes of
both sample size and chunk size (sample size + meta data) and statistical data of `Chunks [/Minute]` is provided as
well. When a publisher port instantly provides data to a subscriber with the `subscribe()` call, the `Field` column is
ticked. The service discovery protocol allows you to define the `Propagation scope` of the data. This can enable
data forwarding to other machines e.g. over network or just consume them internally. When a `Callback` is
registered on subscriber side, the box is ticked accordingly. `FiFo size / capacity` shows the consumption of chunks
on the subscriber side and is a useful column to debug potential memleaks.

    --all             Subscribe to all available introspection data.

`--all` will enable all three views at once.

    -v, --version     Display latest official iceoryx release version and exit.

Make sure that the version number of the introspection exactly matches the version number of RouDi. Currently,
we don't guarantee binary compatibility between different versions. With different version numbers things might break.
