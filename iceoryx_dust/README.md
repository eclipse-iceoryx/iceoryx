
# Eclipse iceoryx dust overview

Similar to iceoryx hoofs `iceoryx_dust` (**D**eemed **u**seful **s**oftware **t**hingies) is a library for basic building blocks.
Compared to hoofs classes in `iceoryx_dust`, the difference is that the classes in dust only conform to quality level 2.

There are a wide variety of building blocks
grouped together in categories or namespace, depending on where or how they are used.

| class/file            | description                                                                                                             |
|:---------------------:|:------------------------------------------------------------------------------------------------------------------------|
|`forward_list`         | Heap and exception free, relocatable implementation of `std::forward_list`                                              |
|`ObjectPool`           | Container which stores raw objects without calling the ctor of the objects.                                             |
|`FileReader`           | Wrapper for opening files and reading them.                                                                             |
|`MessageQueue`         | Interface for Message Queues, see [ManPage mq_overview](https://www.man7.org/linux/man-pages/man7/mq_overview.7.html).  |
|`SignalWatcher`        | Batteries included signal handling with polling and optional blocking wait for `SIGINT` and `SIGTERM`.                  |
|`NamedPipe`            | Shared memory based ipc channel. Mainly a `UnixDomainSocket` replacement on Windows.                                    |
|`relocatable_ptr`      |                                                                                                                         |
|`static_storage`       | Untyped aligned static storage.                                                                                         |
