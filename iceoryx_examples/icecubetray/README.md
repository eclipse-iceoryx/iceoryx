# icecubetray

## Introduction

This example demonstrates how access rights can be applied to shared memory segments on Linux-based operating systems.
It provides a custom RouDi, a radar and a display application.

!!! hint
    The access right feature is only supported on Linux and QNX

## Expected output

<!-- Add asciinema link here -->

## Code walkthrough

RouDi needs to be able to send a _SIGKILL_ signal to the apps in case RouDi is shutdown. Hence, RouDi needs _CAP\_KILL_ capability or similar rights on other POSIX operating system. However, the user _roudi_ does not need root access rights.

| Users        | privileged group | unprivileged group | infotainment group |   iceoryx group    |
|--------------|:----------------:|:------------------:|:------------------:|:------------------:|
| perception   |        X         |                    |                    |         X          |
| infotainment |                  |         X          |         X          |         X          |
| roudi        |                  |                    |                    |         X          |
| notallowed   |                  |                    |                    |         X          |

### Overview over the Apps and Shared Memory Segments

```
                                 +-----------------------+
+--------------------+           |                       |           +---------------------+
|   Radar App        |           | Privileged Shared     |           |  Cheeky App         |
|   @perception      |           | Memory Segment        |           |  @notallowed        |
|                    |  publish  |                       | subscribe |      #     #        |
|            #       |  -------> | r group: unprivileged |  ------>  |       #   #         |
|           #        |           | w group: privileged   |           |        # #          |
|          #         |           |                       |           |        # #          |
|     #   #          |           |                       |  <------  |       #   #         |
|      # #           |           |                       |  publish  |      #     #        |
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
                                 | Infotainment Shared   |           |   Display App       |
                                 | Memory Segment        |  publish  |   @infotainment     |
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

RouDi is built with two static shared memory segments _infotainment_ and _privileged_. The access rights of the segments are configured as depicted in the graphic above.

The `roudiConfig` is composed of a memory pool config called `mepooConfig`. When the segement is created, one needs to
specific the reader group (first string), writer group (second string) as well as the `mepooConfig` (last parameter).
The access rights are solely based on user groups and not on users itself. All users in the reader group are allowed to read, but don't have write access. Users in the writer group have both read and write access.

```cpp
iox::RouDiConfig_t roudiConfig;

// Create Mempool Config
iox::mepoo::MePooConfig mepooConfig;

// We only send very small data, just one mempool per segment
mepooConfig.addMemPool({128, 1000});

/// Create an Entry for a new Shared Memory Segment from the MempoolConfig and add it to the RouDiConfig
roudiConfig.m_sharedMemorySegments.push_back({"unprivileged", "privileged", mepooConfig});
roudiConfig.m_sharedMemorySegments.push_back({"infotainment", "infotainment", mepooConfig});
```

!!! tip
    Shared memory segment can also be configured via a
    [TOML config](https://github.com/eclipse-iceoryx/iceoryx/blob/master/doc/website/advanced/configuration-guide.md#dynamic-configuration) file.

The radar app is started with the user _perception_ and is sending data into the _privileged_ shared memory segment.

The display app is started with the user _infotainment_. It reads the topic `{"Radar", "FrontLeft", "Object"}` from the privileged segement and forwards it as a slighty modified topic `{"Radar", "HMI-Display", "Object"}`. Because the user _infotainment_ only has write access to the infotainment segment, the data is written to this segment.

!!! hint
    It's advised to create per writer group only one shared memory segement (e.g. not two segements with `w: infotainment`).
    In this case it wouldn't be possible to control which segment will be used.

The shared memory segments can be found under `/dev/shm`

```
moss@reynholm:~$ ls -al /dev/shm
total 60268
drwxrwxrwt  2 root  root         100 Apr  7 19:54 .
drwxr-xr-x  6 root  root         460 Apr  6 15:53 ..
-rw-rw----  1 roudi iceoryx 61383328 Apr  7 19:24 iceoryx_mgmt
-rw-rw----+ 1 roudi iceoryx   160000 Apr  7 19:24 infotainment
-rw-rw----+ 1 roudi iceoryx   160000 Apr  7 19:24 privileged
```

!!! note
    Note the shared memory managment segment (`iceoryx_mgmt`) is always available for everyone in the group `iceoryx`
    to **read** and **write**.

#### Not-working setup

The cheeky app is started with the user _notallowed_. It has neither write nor read access to any shared memory segment. Hence, RouDi will print a warning in this case.

Despite having no read access, subscribers can still be created. In this case no data will ever arrive.

```cpp
iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
```

When creating and requesting a publisher RouDi will answer with an error, as there is no write access. Hence, an error will be printed and the cheeky app will stop.

```cpp
iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
```

<center>
[Check out icecubetray on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/icecubetray){ .md-button }
</center>
