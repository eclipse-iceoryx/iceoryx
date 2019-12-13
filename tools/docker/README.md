# Overview

This directory contains files related to building and running [Eclipse iceoryx](https://github.com/eclipse/iceoryx) using [Docker](https://www.docker.com/).

# Building the Docker image and running the container

To create and run a Docker image with a built iceoryx library and examples from your current sources, from the top-level iceoryx repo, run: `./tools/docker/build_and_run.sh`. Or you can navigate to `./tools/docker` and then run `./build_and_run.sh`, per your preference.

For the rest of this text, for brevity, we assume `./tools/docker` is the current directory.

Expected output of `./build_and_run.sh` looks like:

```
Sending build context to Docker daemon  3.352MB
Step 1/8 : FROM ubuntu:bionic
 ---> 4c108a37151f
Step 2/8 : ARG REVISION
 ---> Using cache
 ---> a398e6040568
Step 3/8 : ARG B_ICEORYX_BUILD
 ---> Using cache
 ---> 6adec4590084
Step 4/8 : ENV ICEORYX_BUILD=$B_ICEORYX_BUILD
 ---> Using cache
 ---> c0ff19e371ca
Step 5/8 : RUN apt update && apt install -y         cmake         libacl1-dev         libncurses5-dev         pkg-config         screen         git
 ---> Using cache
 ---> 6c0d2d84c2f9
Step 6/8 : ADD . /
 ---> 65602097702b
Step 7/8 : WORKDIR /iceoryx
 ---> Running in 61ce7c4b3718
Removing intermediate container 61ce7c4b3718
 ---> 987569d9f854
Step 8/8 : RUN ./tools/iceoryx_build_test.sh     && cp ./tools/docker/.screenrc /root
 ---> Running in 28767c2c7c9f
 [i] Create a new build directory and change the current working directory
 [i] Current working directory:
/iceoryx/build
>>>>>> Start building iceoryx utils package <<<<<<
-- The C compiler identification is GNU 7.4.0
-- The CXX compiler identification is GNU 7.4.0
-- Check for working C compiler: /usr/bin/cc
...
[100%] Linking CXX executable icedelivery/ice_receiver_simple
[100%] Built target ice_receiver_simple
>>>>>> finished building iceoryx examples <<<<<<
Removing intermediate container 28767c2c7c9f
 ---> ab28011a1a6b
Successfully built ab28011a1a6b
Successfully tagged iceoryx:master
```

After the Docker image is built and tagged, a corresponding container is launched, and you are dropped in a shell inside the `iceoryx` directory containing the library source and binaries.

# Launching the RouDi daemon and example applications inside the container

The next step is to launch the RouDi daemon to enable communication and run example communicating applications.

## Using `docker exec`

After using `./build_and_run.sh`, you can launch the RouDi daemon in the provided shell:

```
$ ./build_and_run.sh
...
root@03043f96ae10:/iceoryx# ./build/install/prefix/bin/RouDi
Reserving 95306080 bytes in the shared memory [/iceoryx_mgmt]
[ Reserving shared memory successful ]
Reserving 595259200 bytes in the shared memory [/root]
[ Reserving shared memory successful ]
```

then, in a new terminal, issue `docker ps` to see running containers:

```
$ docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED              STATUS              PORTS               NAMES
03043f96ae10        iceoryx:current     "/bin/bash"         About a minute ago   Up About a minute                       roudi
```

and start the sender application:

```
badc0ded@localhost:~$ docker exec -it roudi /bin/bash
root@03043f96ae10:/iceoryx# ./build/iceoryx_examples/icedelivery/ice_sender
Sending: 0
Sending: 1
Sending: 2
Sending: 3
```

and in another terminal, start the receiver application:

```
$ docker exec -it roudi /bin/bash
root@03043f96ae10:/iceoryx# ./build/iceoryx_examples/icedelivery/ice_receiver
Not subscribed
Receiving: 78
Receiving: 79
Receiving: 80
Receiving: 81
```

## Using `screen` to launch RouDi and examples

Another way of interacting with the daemon and examples is to use [screen](https://www.gnu.org/software/screen/) (also included in the Docker image) to start executables in different virtual screens that you attach to and detach from as your experiments may require.

This may be a better option if you are working on a remote machine and don't want to have to open multiple connections to it in separate terminals and/or if you're comfortable with using `screen`.

To prepare a virtual screen in which we'll launch the RouDi daemon, enter:

```
$ ./build_and_run.sh
...
root@03043f96ae10:/iceoryx# screen
```

and press `[Enter]`.

Inside the virtual screen shell, you can then launch the RouDi daemon as follows:

```
root@3b93f0d3eda2:/iceoryx# ./build/install/prefix/bin/RouDi
Reserving 95306080 bytes in the shared memory [/iceoryx_mgmt]
[ Reserving shared memory successful ]
Reserving 595259200 bytes in the shared memory [/root]
[ Reserving shared memory successful ]
```

and then press `[Ctrl]+A D` to detach from the virtual screen.

You should a see a message like this:
```
[detached from 16.pts-0.3b93f0d3eda2]
```

To launch an example sender application, start a new virtual screen and run the corresponding binary:

```
root@3b93f0d3eda2:/iceoryx# screen
#[Enter]
root@3b93f0d3eda2:/iceoryx# ./build/iceoryx_examples/icedelivery/ice_sender
Sending: 0
Sending: 1
Sending: 2
Sending: 3
Sending: 4
...
```

As above, press `[Ctrl]+A D` to detach from this virtual screen.

Then the receiver application can be started, again in a separate virtual screen:

```
root@3b93f0d3eda2:/iceoryx# screen
#[Enter]
root@3b93f0d3eda2:/iceoryx# ./build/iceoryx_examples/icedelivery/ice_receiver
Not subscribed
Receiving: 12
Receiving: 13
Receiving: 14
Receiving: 15
...
```

You can now detach from this virtual screen with `[Ctrl]+A D`, and, as an exercise, list all screens and return to the one with the sender.

```
root@d3e51ca29d56:/iceoryx# screen -r
There are several suitable screens on:
	66.pts-0.d3e51ca29d56	(12/06/19 08:55:00)	(Detached)
	35.pts-0.d3e51ca29d56	(12/06/19 08:54:48)	(Detached)
	17.pts-0.d3e51ca29d56	(12/06/19 08:53:57)	(Detached)
Type "screen [-d] -r [pid.]tty.host" to resume one of them.
```

Try the following to return to the sender application (adjust the PID as per your exact `screen -r` output above):

```
screen -r 35
```

You should see again the virtual screen of the sender application:

```
Sending: 212
Sending: 213
Sending: 214
Sending: 215
Sending: 216
...
```

You can stop the sender with `[Ctrl]+C`, and then detach from this screen with `[Ctrl]+A D`, and return to the receiver screen with (adjusting the PID as per your `screen -r` output above):

```
screen -r 66
```

and you should see current receiver output such as:

```
Not subscribed
Not subscribed
Not subscribed
Not subscribed
Not subscribed
...
```

As an exercise, try return to the sender screen and relaunch the sender, then detach from the sender screen and attach to the receiver screen and see the output based on the new sender:

```
Receiving: 103
Receiving: 104
Receiving: 105
Receiving: 106
Receiving: 107
...
```
