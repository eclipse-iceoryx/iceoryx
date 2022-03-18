# Use Iceoryx In A Docker Environment

## Introduction

Let's assume we are working on a system in which `iox-roudi` runs in a docker
environment and it should orchestrate two applications which are running again
in two different docker containers so that we end up with a system of 3
different docker containers.

To demonstrate the setup we use the [icedelivery C++ example](../icedelivery).

```
                         +-----------+
                         | docker 1  |
                         |           |
                         | iox-roudi |
                         +-----------+

       +-------------------+        +--------------------+
       | docker 2          | send   | docker 3           |
       |                   |------->|                    |
       | iox-cpp-publisher | data   | iox-cpp-subscriber |
       +-------------------+        +--------------------+
```

## Requirements

### Shared Access to Unix Domain Sockets

Every iceoryx application registers itself at our central broker RouDi
by sending a message to the unix domain socket located at
`IOX_UDS_SOCKET_PATH_PREFIX/roudi` which is defined in the corresponding
platform settings file `platform_settings.hpp`. In linux the socket file handle
can be found at `/tmp/roudi`. When the application registers at RouDi it
announces its unix domain socket as well to receive responses of requests which
will be sent during runtime to RouDi.
This socket is stored as well in `/tmp/IOX_RUNTIME_NAME`. The `iox-cpp-publisher`
runtime has the same name as the binary which leads to the socket
`/tmp/iox-cpp-publisher`.

### Shared Access to File Locks

Iceoryx applications ensure that every runtime name is unique in the system
by creating a file lock before creating the runtime. This is stored in
`IOX_LOCK_FILE_PATH_PREFIX/IOX_RUNTIME_NAME.lock` whereby
`IOX_LOCK_FILE_PATH_PREFIX` is defined in the platform settings file
`platform_settings.hpp`. When running the icedelivery example in a linux
environment one can observe
the lock files `/tmp/roudi.lock`, `/tmp/iox-cpp-subscriber.lock` and
`/tmp/iox-cpp-publisher.lock`.

### Shared Access to Semaphores and Shared Memory

One of the tasks of the central broker RouDi is to create and distribute shared
memory. When the `iox-cpp-publisher` would like to send data it acquires a
pointer to this shared memory, writes the data into it and sends the
pointer to the `iox-cpp-subscriber` which reads the memory at the received
memory position.
Additionally, it is possible to signal events across process boundaries via
semaphores. For instance to signal a subscriber that data has arrived.

## Implementation

To have shared access to the required resources we have to bind the host
filesystem:

 * `/tmp`
 * `/dev`

into every docker container.

### Terminal Example

We start in 3 separate terminals 3 docker instances. In this example we
use `archlinux:latest` but one is free to choose any other linux distribution.
The iceoryx repository which contains an already built iceoryx can be found at
`/home/user/iceoryx` which is bound to `/iceoryx`. The usage is
explained in detail in the [icedelivery C++ example](../icedelivery).

#### Terminal 1 (iox-roudi)
```
docker run --mount type=bind,source="/dev",target=/dev --mount type=bind,source=/home/user/iceoryx,target=/iceoryx --mount type=bind,source=/tmp,target=/tmp -it archlinux:latest

cd /iceoryx
./build/iox-roudi
```

#### Terminal 2 (iox-cpp-publisher)
```
docker run --mount type=bind,source="/dev",target=/dev --mount type=bind,source=/home/user/iceoryx,target=/iceoryx --mount type=bind,source=/tmp,target=/tmp -it archlinux:latest

cd /iceoryx
./build/iceoryx_examples/icedelivery/iox-cpp-publisher
```

#### Terminal 3 (iox-cpp-subscriber)

```
docker run --mount type=bind,source="/dev",target=/dev --mount type=bind,source=/home/user/iceoryx,target=/iceoryx --mount type=bind,source=/tmp,target=/tmp -it archlinux:latest

cd /iceoryx
./build/iceoryx_examples/icedelivery/iox-cpp-subscriber
```

### docker-compose Example

We can also use `docker-compose` to start our test setup. Our example is coming
with a configuration file `docker-compose.yml` which can be used from the
iceoryx root path with the following command:

```
docker-compose -f iceoryx_examples/icedocker/docker-compose.yml --project-directory . up
```

We have to set the project directory explicitly so that the mapping of the
iceoryx root path is working as intended.
