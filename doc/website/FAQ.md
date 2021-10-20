# FAQ

In this document are tips and hints documented which can help for troubleshooting on RouDi.

## Does iceoryx run in a docker environment

Yes. Take a look at the [icedocker example](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/icedelivery)

## iox-roudi fails on startup

An error message like

```
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

```sh
df -H /dev/shm
```

if you have enough memory available. In this exemplary error message we require
`59736448` (iceoryx management data) + `27902400` (user samples) ~ `83.57mb`
of shared memory.

### docker

When you are in a docker environment check if there is enough memory available
in your docker.

```
# docker stats
CONTAINER ID   NAME            CPU %     MEM USAGE / LIMIT   MEM %     NET I/O       BLOCK I/O     PIDS
367b9fae6c2f   nifty_galileo   0.00%     4.48MiB / 1GiB      0.44%     11.6kB / 0B   17.6MB / 0B   1
```

If not you can restart the docker container with `--shm-size="2g"` to increase
the total amount of available shared memory.

```
docker run -it --shm-size="2g" ubuntu
```

## Termination of RouDi and processes

To avoid undefined behavior of iceoryx posh it is recommended to terminate RouDi and the corresponding middleware
processes with SIGINT or SIGTERM. In RouDi, we have integrated a sighandler that catches the signals and gives RouDi
the chance to exit and clean-up everything. This also applies for processes. Therefore, we recommend adding a signalhandler
to your process (see [this example](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/icedelivery/iox_publisher_untyped.cpp)).
