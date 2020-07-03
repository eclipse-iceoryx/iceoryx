# Overview

This directory contains files related to building and running [Eclipse iceoryx](https://github.com/eclipse/iceoryx) using [Docker](https://www.docker.com/).

# Building the iceoryx Docker Image

To create a Docker image with the built iceoryx libraries and examples from your current sources, run the following script from the root of the repository: `./tools/docker/build_roudi_docker.sh`.

After the Docker image is built, you can run it in interactive mode then use one of the below options to connect to it and run the example applications.

# Running an iceoryx Docker container

A helper script is provided to launch containers for ready-built containers.
Simply run the following script from any location:
```
./tools/docker/run_roudi_docker.sh
```

# Connecting to the Docker container and running the example applications

With the iceoryx Docker container running RouDi, you can connect to it to play with the example applications.
There are a couple of methods available to connect to do this.

## Using `docker exec`

It is possible to bind a shell to running containers via `docker exec`. A helper script is provided with the command for convenience.
To bind a shell to the container, simply run the following script from any location:
```
./tools/docker/bind_roudi_docker.sh
```
You will then be dropped into a bash shell where you can access all iceoryx binaries.

All iceoryx binaries are directly available in the $PATH of the shell.
For example, to start the sender application, you need only run the following from the bound bash shell:

```
root@b10b3630f6d3:/# ice-publisher-simple 
2020-07-02 16:18:58.811 [ Debug ]: Application registered management segment 0x7f9c7fbc4000 with size 71546016 to id 1
2020-07-02 16:18:58.811 [ Info  ]: Application registered payload segment 0x7f9c76d0a000 with size 149655680 to id 2
Sending: 0
Sending: 1
Sending: 2
```

To run the receiver application, bind another shell to the container and run the following:

```
root@b10b3630f6d3:/# ice-subscriber-simple 
2020-07-02 16:21:00.242 [ Debug ]: Application registered management segment 0x7f9d8fbc4000 with size 71546016 to id 1
2020-07-02 16:21:00.242 [ Info  ]: Application registered payload segment 0x7f9d86d0a000 with size 149655680 to id 2
Callback: 1
Callback: 2
Callback: 3
```

The complete communication flow should now be observable.

## Using `screen`

Another way of interacting with the daemon and examples is to use [screen](https://www.gnu.org/software/screen/) (also included in the Docker image) to start executables in different virtual screens that you attach to and detach from as your experiments may require.

This may be a better option if you are working on a remote machine and don't want to have to open multiple connections to it in separate terminals and/or if you're comfortable with using `screen`.

To start a session with screen, run the following from a bound bash shell:

```
root@b10b3630f6d3:/# screen
```

and press `[Enter]`.

Inside the virtual screen shell, you can then launch the example applications.
To launch an example sender application, run the corresponding binary:

```
root@3b93f0d3eda2:/iceoryx# screen
#[Enter]
root@3b93f0d3eda2:/iceoryx# ice-publisher-simple
Sending: 0
Sending: 1
Sending: 2
Sending: 3
Sending: 4
...
```

Press `[Ctrl]+A D` to detach from this virtual screen.

Then, the receiver application can be started in a separate virtual screen:

```
root@3b93f0d3eda2:/iceoryx# screen
#[Enter]
root@3b93f0d3eda2:/iceoryx# ice-subscriber-simple
Not subscribed
Receiving: 12
Receiving: 13
Receiving: 14
Receiving: 15
...
```

You can now detach from this virtual screen with `[Ctrl]+A D`.

As an exercise, you can return to previous screen sessions to observe their current output.
For examples, here we list all screens and return to the one running the sender:

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
