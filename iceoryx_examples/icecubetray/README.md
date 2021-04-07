# icecubetray

## Introduction

This example demonstrates how access rights can be applied to shared memory segments.
It provides a custom RouDi, a radar and a display application.

## Expected output

<!-- Add asciinema link here -->

## Code walkthrough

The user _roudi_ does not need root access rights. However, it needs _CAP\_KILL_ capability or something similar on
other POSIX operating system. RouDi needs to be able to send a _SIGKILL_ signal to the apps in case RouDi is shutdown.

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

#### RouDi and Apps


##### Working setup

RouDi is built with two static shared memory segments _infotainment_ and _privileged_. The access rights of the segments are configured as depicted in the graphic above.

The radar app is started with the user _perception_ and is sending data into the _privileged_ shared memory segment.

The display app is started with the user _infotainment_. It reads the topic `{"Radar", "FrontLeft", "Object"}` from the privileged segement and forwards it as a slighty modified topic `{"Radar", "HMI-Display", "Object"}`. Because the user _infotainment_ only has write access to the infotainment segment, the data is written to this one.

!!! hint
    It's advised to create per writer group only one shared memory segement (e.g. not two segements with `w: infotainment`).
    In this case it wouldn't be possible to control which segment will be used. (=> add a test for that)

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
    Note the shared memory managment segment is always available for everyone to **read** and **write**

##### Not-working setup

The cheeky app is started with the user __notallowed_. It has neither write nor read access to any shared memory segment. Hence, RouDi will print a warning in this case.

(Start preprocessing app with 'notallowed' user, should not get not_null segfault in RouDi, but something like "Access denied, no shared memory payload segment available")
