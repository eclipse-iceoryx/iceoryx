# FAQ

In this document are tips and hints documented which can help for troubleshooting on RouDi.

## Does iceoryx run in a docker environment?

Yes. Take a look at the [icedocker example](../../iceoryx_examples/icedocker/)

## iceoryx crashes with SIGABRT when reserving shared memory in a docker envirnonment

Check the `--shm-size` flag of the docker container. Does the container provide the shared memory size required for the segment?

## How can I find out if RouDi is running?

RouDi uses a file locking machanism to ensure that only one RouDi instance is running at a time. For that RouDi
creates and locks `/tmp/roudi.lock`. The file exists also also when RouDi is not running. Try locking this file,
if this fails RouDi is running.

On the command line do

```console
flock -n /tmp/roudi.lock echo "RouDi is not running"
```

## Missing samples in a publish subscribe scenario

Lost samples are not acceptable for your system? Is your publisher sending at a higher frequency than the subscribers
are taking the samples?

Either

* Make sure that the receiving frequency is higher than the publishing one

or

* Use the [blocking publisher feature](../../iceoryx_examples/iceoptions/)

!!! caution
    The usage of the blocking publisher feature needs to be considered carefully as other subscribers will not receive
    samples while the publisher is blocked.

A possible alternative is

* Increase `SubscriberOptions::queueCapacity` to up to 256
  * If 256 is not enough, increase the maximum value `IOX_MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY`
  via [the CMake switch](advanced/configuration-guide.md)

## Missing samples with a Listener

In case the subscriber is used in combination with a listener, some samples might just wait in receiver queue to be taken.
The `Listener` uses events, which are faster than states of the `WaitSet` but this also means that if the publisher e.g.
fires 5 events while the subscriber is executing the `onSampleReceivedCallback` it will be triggered only once after it
leaves the callback. If you do not take care of taking all the data out of the queue, they will just stay there and fill
up the queue. The default queue size is 256 samples. This means the samples need to be taken out in a loop in the
`onSampleReceivedCallback` until `take` reports an empty queue. Alternatively the `WaitSet` can be used instead of the `Listener`.
The `WaitSet` supports states as well as events and if used with states it will fire as long as there are data in the queue.

## Solving the error `MEPOO__MEMPOOL_GETCHUNK_POOL_IS_RUNNING_OUT_OF_CHUNKS`

Possible solutions are one of the following:

1. Increase [memory configuration of RouDi](advanced/configuration-guide.md)
1. Make sure that the receiving frequency is higher than the publishing one
1. Reduce `SubscriberOptions::queueCapacity` to hold less samples in the mempool on the subscriber side
1. Consider using the [blocking publisher feature](../../iceoryx_examples/iceoptions/). The usage needs to be
considered carefully as other subscribers will not receive samples while the publisher is blocked.

!!! caution
    The usage of the blocking publisher feature needs to be considered carefully as other subscribers will not receive
    samples while the publisher is blocked.

## iox-roudi fails on startup

An error message like

```console
user@iceoryx-host:/# iox-roudi
Log level set to: [Warning]
SharedMemory still there, doing an unlink of /iceoryx_mgmt
Reserving 59736448 bytes in the shared memory [/iceoryx_mgmt]
[ Reserving shared memory successful ]
SharedMemory still there, doing an unlink of /root
Reserving 27902400 bytes in the shared memory [/root]
While setting the acquired shared memory to zero a fatal SIGBUS signal appeared
caused by memset. The shared memory object with the following properties
[ name = /root, sizeInBytes = 27902400, access mode = AccessMode::READ_WRITE,
ownership = OwnerShip::MINE, baseAddressHint = (nil), permissions = 0 ] maybe
requires more memory than it is currently available in the system.
```

indicates that there is not enough shared memory available. Check with

```console
df -H /dev/shm
```

if you have enough memory available. In this exemplary error message we require
`59736448` (iceoryx management data) + `27902400` (user samples) ~ `83.57mb`
of shared memory.

## Stack size handling

The iceoryx middleware utilize stack memory from the system for book-keeping of
internal structures.
Most Linux distributions offers 8 Megabyte of stack memory for a process which is enough
for iceoryx. You can check this with the output from `ulimit -a`.

On other platforms like windows other rules apply for the stack memory.
On windows there is [only 1 Megabyte](https://docs.microsoft.com/en-us/cpp/build/reference/stack-stack-allocations?view=msvc-170) of stack available.
Increasing the stack size generally on iceoryx is not recommended since `Roudi`
could consume lots of memory without using it.
Especially using RouDi in a multi-threaded context can run out the stack memory and
lead to memory errors.

The `Single process` example shows that when compiling and executing it on windows.
Without setting the stack size the application will throw a `Stack overflow` exception
when entering the `main()` method.

This can be solved in CMake by adding a linker flag:

```cmake
target_link_options(single_process BEFORE PRIVATE /STACK:3500000)
```

For other platforms apply other flags or solutions.

### CI fails but error is locally not reproducable

One can use `tools/scripts/ice_env.sh` to create an iceoryx development environment
with a configuration very similar to the CI target.
When for instance the target ubuntu 18.04 fails one can create a docker container
with

```sh
cd tools/scripts
./ice-env.sh enter ubuntu:18.04
```

This starts the container, installs all dependencies which iceoryx requires and enters
the environment.

### docker

When you are in a docker environment check if there is enough memory available
in your docker.

```console
# docker stats
CONTAINER ID   NAME            CPU %     MEM USAGE / LIMIT   MEM %     NET I/O       BLOCK I/O     PIDS
367b9fae6c2f   nifty_galileo   0.00%     4.48MiB / 1GiB      0.44%     11.6kB / 0B   17.6MB / 0B   1
```

If not you can restart the docker container with `--shm-size="2g"` to increase
the total amount of available shared memory.

```console
docker run -it --shm-size="2g" ubuntu
```

## Termination of RouDi and processes

To avoid undefined behavior of iceoryx posh it is recommended to terminate RouDi and the corresponding middleware
processes with SIGINT or SIGTERM. In RouDi, we have integrated a sighandler that catches the signals and gives RouDi
the chance to exit and clean-up everything. This also applies for processes. Therefore, we recommend adding a signalhandler
to your process (see [this example](../../iceoryx_examples/icedelivery/iox_publisher_untyped.cpp)).

## How to use iceoryx as external dependency with bazel

Define iceoryx repository information in your [WORKSPACE](https://bazel.build/concepts/build-ref#workspace)
then calling bazel macro from [load_repositories.bzl](https://github.com/eclipse-iceoryx/iceoryx/blob/main/bazel/load_repositories.bzl)
and [setup_repositories.bzl](https://github.com/eclipse-iceoryx/iceoryx/blob/main/bazel/setup_repositories.bzl) for loading transitive dependencies.

```
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

IOX_COMMIT = "....."

http_archive(
    name = "eclipse_iceoryx",
    sha256 = <sha256 sum of z>,
    strip_prefix = "iceoryx-" + IOX_COMMIT,
    url = "https://github.com/eclipse-iceoryx/iceoryx/archive/" + IOX_COMMIT + ".zip",
)

# load iceoryx transitive dependencies

load("@eclipse_iceoryx//bazel:load_repositories.bzl", "load_repositories")
load("@eclipse_iceoryx//bazel:setup_repositories.bzl", "setup_repositories")
load_repositories()
setup_repositories()

```
