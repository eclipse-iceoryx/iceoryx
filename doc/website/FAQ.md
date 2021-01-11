# FAQ

In this document are tips and hints documented which can help for troubleshooting on RouDi.

## Available memory is insufficient

If you got this message, RouDi was not able to reserve shared memory caused by insufficient capacity.
To avoid that you need to check how much shared memory your system offers. For example, on Ubuntu you can do that with

    df -H /dev/shm

## Termination of RouDi and processes

To avoid undefined behavior of iceoryx posh it is recommended to terminate RouDi and the corresponding middleware
processes with SIGINT or SIGTERM. In RouDi, we have integrated a sighandler that catches the signals and gives RouDi
the chance to exit and clean-up everything. This also applies for processes. Therefore, we recommend adding a signalhandler
to your process (see [this example](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/icedelivery/iox_publisher_untyped.cpp)).
