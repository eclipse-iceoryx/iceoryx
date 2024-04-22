# ice_access_control

## Introduction

This example demonstrates how access rights can be applied to shared memory segments on Linux-based operating systems.
It provides a custom RouDi, a radar and a display application.

!!! hint
    The access right feature is only supported on Linux and QNX

## Expected output

[![asciicast](https://asciinema.org/a/407451.svg)](https://asciinema.org/a/407451)

## Code walkthrough

RouDi needs to be able to send a _SIGKILL_ signal to the apps in case RouDi shuts down. Hence, RouDi needs
_CAP\_KILL_ capability or similar rights on other POSIX operating systems. However, the user _roudi_ does
not need root access rights.

The system user can be member of multiple system groups. This examples uses the following users and groups:

| Users        | privileged group | unprivileged group | infotainment group |   iceoryx group    |
|--------------|:----------------:|:------------------:|:------------------:|:------------------:|
| perception   |        X         |                    |                    |         X          |
| infotainment |                  |         X          |         X          |         X          |
| roudi        |                  |                    |                    |         X          |
| notallowed   |                  |                    |                    |         X          |

!!! hint
    In order to be able to use iceoryx communication, all apps have to be in the group _iceoryx_.

### Overview over the Apps and Shared Memory Segments

RouDi is built with two shared memory segments _infotainment_ and _privileged_. On startup, it creates these two shared
memory segments in the operating system. The _privileged_ segment requires the app to be started with the group
_privileged_ if it publishes data or with the _unprivileged_ group when data should only be received.
The _infotainment_ segment on the other hand requires only one group for reading and writing called _infotainment_.
See the [next chapter](#working-setup) for a detailed description on how to configure the shared memory segments.

```
                                 +-----------------------+
+--------------------+           |                       |           +---------------------+
|  Radar App         |           | Privileged Shared     |           |  Cheeky App         |
|  user: perception  |           | Memory Segment        |           |  user: notallowed   |
|                    |  publish  |                       |           |      #     #        |
|            #       |  -------> | r group: unprivileged |           |       #   #         |
|           #        |           | w group: privileged   |           |        # #          |
|          #         |           |                       |           |        # #          |
|     #   #          |           |                       |           |       #   #         |
|      # #           |           |                       |           |      #     #        |
+--------------------+           |                       |           +---------------------+
                                 +-----------------------+
                                                 |
                                                 |          subscribe
                                                 +---------------------------+
                                                                             |
                                                                             |
                                                                             |
                                                                            \ /
                                 +-----------------------+
                                 |                       |           +---------------------+
                                 | Infotainment Shared   |           |  Display App        |
                                 | Memory Segment        |  publish  |  user: infotainment |
                                 |                       |  <------  |                     |
                                 | r group: infotainment |           |            #        |
                                 | w group: infotainment |           |           #         |
                                 |                       |           |          #          |
                                 |                       |           |     #   #           |
                                 |                       |           |      # #            |
                                 |                       |           +---------------------+
                                 +-----------------------+
```

### RouDi and Apps

#### Working setup

Do the following to configure shared memory segments when building a custom RouDi:

<!--[geoffrey][iceoryx_examples/ice_access_control/roudi_main_static_segments.cpp][config]-->
```cpp
iox::IceoryxConfig config;
static_cast<iox::config::RouDiConfig&>(config) = cmdLineArgs.value().roudiConfig;


// Create Mempool Config
iox::mepoo::MePooConfig mepooConfig;

// We only send very small data, just one mempool per segment
mepooConfig.addMemPool({128, 1000});

// Create an entry for a new shared memory segment from the mempooConfig and add it to the iceoryx config
// Parameters are {"ReaderGroup", "WriterGroup", MemoryPoolConfig}
config.m_sharedMemorySegments.push_back({"unprivileged", "privileged", mepooConfig});
config.m_sharedMemorySegments.push_back({"infotainment", "infotainment", mepooConfig});
```

The `config` is composed of a memory pool config called `mepooConfig`. When the segment is created, one needs to
specify the reader group (first string), writer group (second string) as well as the `mepooConfig` (last parameter).
The access rights are solely based on user groups and not on users itself. All users in the reader group are allowed
to read, but don't have write access. Users in the writer group have both read and write access.

!!! tip
    Shared memory segment can also be configured via a
    [TOML config](../../doc/website/advanced/configuration-guide.md#dynamic-configuration) file.

The radar app is started with the user _perception_, which is in the group _privileged_. Therefore it has write access
to the _privileged_ segment and is sending data into the _privileged_ shared memory segment.

The display app is started with the user _infotainment_, which is in the group _infotainment_ and _unprivileged_.
Therefore it has read access to the _privileged_ segment. It reads the topic `{"Radar", "FrontLeft", "Object"}` from
the _privileged_ segment and forwards it as a slighty modified topic `{"Radar", "HMI-Display", "Object"}`. Because
the user _infotainment_ is only in the _infotainment_ and _unprivileged_ group, it only has write access to the
infotainment segment. Hence, the data is written to this segment.

!!! hint
    It's advised to create only one shared memory segment per writer group (e.g. not two segments with `w: infotainment`).
    In this case it wouldn't be possible to control which segment will be used.

The shared memory segments can be found under `/dev/shm`

```
moss@reynholm:$ getfacl /dev/shm/*
# file: dev/shm/iceoryx_mgmt
# owner: roudi
# group: iceoryx
user::rw-
group::rw-
other::---

# file: dev/shm/infotainment
# owner: roudi
# group: iceoryx
user::rw-
group::rw-
group:infotainment:rw-
mask::rw-
other::---

# file: dev/shm/privileged
# owner: roudi
# group: iceoryx
user::rw-
group::rw-
group:privileged:rw-
group:unprivileged:r--
mask::rw-
other::--
```

!!! note
    Note the shared memory managment segment (`iceoryx_mgmt`) is always available for everyone in the group `iceoryx`
    to **read** and **write**.

#### Not-working setup

The cheeky app is started with the user _notallowed_. This user is not in any group that allow either read
or write access to any of the shared memory segments. Hence, RouDi will print a warning in this case.

Despite having no read access, subscribers can still be created. <!-- In this case no data will ever arrive.-->

<!--[geoffrey][iceoryx_examples/ice_access_control/iox_cheeky_app.cpp][subscriber]-->
```cpp
iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
```

When creating and requesting a publisher, RouDi will answer with an error, as there is no write access. Hence,
an error will be printed and the cheeky app will stop.

<!--[geoffrey][iceoryx_examples/ice_access_control/iox_cheeky_app.cpp][publisher]-->
```cpp
iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
```

<center>
[Check out ice_access_control on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/ice_access_control){ .md-button } <!--NOLINT github url required for website-->
</center>
