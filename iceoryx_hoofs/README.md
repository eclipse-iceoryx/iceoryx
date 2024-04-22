
# Eclipse iceoryx hoofs overview

The iceoryx hoofs (**H**andy **O**bjects **O**ptimised **F**or **S**afety) are our basic
building blocks - the foundation of iceoryx. There are a wide variety of building blocks
grouped together in modules, depending on where or how they are used.

## Modules

The following sections have a column labeled `internal` to indicate that the API
is not stable and can change anytime. You should never rely on it and there is no
support if it is used and breaks your code after an update.

Some modules contain STL constructs which are not part of the C++17 standard as well as convenience
constructs like the `NewType`. Since the classes re-implements some STL constructs,
the C++ STL coding guidelines are used for all files in this module, to help the user
to have a painless transition from the official STL types to ours.
The API should also be identical to the corresponding STL types but we have to make
exceptions here. For instance, we do not throw exceptions, try to avoid undefined behavior
and we do not use dynamic memory. In these cases we adjusted the API to our use case.

Most of the headers are providing some example code on how the
class should be used.

The module structure is a logical grouping. It is replicated for `concurrent` and `posix` implementations.

### Memory & lifetime management (memory)

| class                              | internal | description                                                                                                                                                                                                              |
|:----------------------------------:|:--------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`unique_ptr`                        |          | Provides a heap-less unique_ptr implementation, unlike the STL                                                                                                                                                           |
|`RelativePointer`                   |          | Pointer which can be stored in shared memory                                                                                                                                                                             |
|`relocatable_ptr`                   |          |                                                                                                                                                                                                                                                                                                            |
|`static_storage`                    | i        | Untyped aligned static storage.                                                                                                                                                                                                                                                                                       |
|`ScopeGuard`                        |          | This is an abstraction of the C++ RAII idiom. Sometimes you have constructs where you would like to perform a certain task on creation and then again when they are getting out of scope, this is where `ScopeGuard` comes in. It is like a `std::lock_guard` or a `std::shared_ptr` but more generic. |
|`scoped_static`                     |          | Helper function to limit lifetime of static or global variables to a scope                                                                                                                                               |
|`BumpAllocator`                     |          | Implementation of a bump allocator                                                                                                                                                                                       |

### Container (container)

| class                 | internal | description                                                                                                                                                                                                                           |
|:---------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`vector`               |          | Heap and exception free implementation of `std::vector`                                                                                                                                                                               |
|`forward_list`         |          | Heap and exception free, relocatable implementation of `std::forward_list`                                                                                                                                                                                                                                            |
|`list`                 |          | Heap and exception free, relocatable implementation of `std::list`                                                                                                                                                                    |
|`FixedPositionContainer` |        | A fixed-position container is similar to a list but is optimized for iterating over its elements without the back-and-forth jumping that can occur during iteration in a list. |
|`UninitializedArray`   |          | Wrapper class for an uninitialized C-style array which can be zeroed via a template parameter                                                                                                                                         |

### Vocabulary types (vocabulary)

| class                 | internal | description                                                                                                                                                                                                                           |
|:---------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`not_null`             |          | Runtime check wrapper to make sure a pointer is not `nullptr`                                                                                                                                                                         |
|`optional`             |          | Implementation of `std::optional`                                                                                                                                                                             |
|`variant`              |          | Implementation of `std::variant`                                                                                                                                                                              |
|`expected`             |          | Our base class used in error handling. Every function which can fail should return an expected. With this the user knows that this function can fail and that they have to do some kind of error handling. We got inspired by the [C++ expected proposal]( http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0323r7.html) and by the [rust error handling concept](https://doc.rust-lang.org/std/result/enum.Result.html). |
|`string`               |          | Heap and exception free implementation of `std::string`. Attention, since the string is stack based, std::string or char array which are assigned to this string will be truncated and zero-terminated if they exceed the string capacity. |
|`span`                 |          | Implementation of `std::span`                                                                                                                                     |

### Filesystem (filesystem)

| class                 | internal | description                                                                                                                                                                                                                           |
|:---------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`filesystem`           |          | Implementation of C++17 filesystem features for instance `iox::access_rights` and `iox::perms` to abstract file permissions                                                                                                                                    |
|`PosixAcl`             | i        | Interface for Access Control Lists (ACL).                                                                                                                                                                                             |
|`FileLock`             |          | File lock C++ wrapping class.                                                                                                                                                                                                         |
|`FileReader`           |          | Wrapper for opening files and reading them.                                                                                                                                                                                                                                                                           |

### Functional (functional)

| class                 | internal | description                                                                                                                                                                                                                            |
|:---------------------:|:--------:|:---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`storable_function`    | i        | A `std::function` alternative with configurable backend for memory storage.                                                                                                                                                            |
|`function`             |          | A stack-based `std::function` replacement based on `storable_function`                                                                                                                                                                 |
|`function_ref`         |          | Implementation of `std::function_ref` see [function_ref proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0792r2.html). It behaves like `std::function` but does not own the callable. |

### Utility (utility)

| class                 | internal | description                                                                                                                                                                                                                                |
|:---------------------:|:--------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`system_configuration` | i        | Collection of free functions which acquire system information like the page-size.                                                                                                                                                          |
|`UniqueId`             | i        | Monotonic increasing IDs within a process.                                                                                                                                                                                                 |
|`convert`              | i        | Converting a number into a string is easy, converting it back can be hard. You can use functions like `strtoll`, but you still have to handle errors like under- and overflow, or converting invalid strings into number. Here we abstract all the error handling so that you can convert strings into numbers safely. |
|`into`                 | i        |                                                                                                                                                                                                                                            |
|`Scheduler`            | i        | Supported schedulers and functions to get their priority range are contained here.                                                                                                                                                    |
|`serialization`        | i        | Implements a simple serialization concept for classes based on the idea presented here [ISOCPP serialization](https://isocpp.org/wiki/faq/serialization#serialize-text-format). |

### Primitives (primitives)

| class                 | internal | description                                                                                                                                                                                                                           |
|:---------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`type_traits`          |          | Extended support for evaluating types on compile-time.                                                                                                                                                                                |
|`types`                |          | Declares essential building block types like `byte`.                                                                                                                                                                                |
|`attributes`           |          | C++20 and C++23 attributes are sometimes available through compiler extensions. The attribute macros defined in here make sure that we are able to use them if the compiler supports it. |
|`algorithm`            |          | Implements `min` and `max` for an arbitrary number of values of the same type. For instance `min(1,2,3,4,5);`                                                                                                                         |
|`size`                 |          | Helper functions to determine the size in generic ways                                                                                                                                                                                |

### Buffer (buffer)

| class                              | internal | description                                                                                                                                                                                                                        |
|:----------------------------------:|:--------:|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`SpscFifo`                          | i        | Single producer, single consumer lock-free FiFo                                                                                                                                                                                    |
|`MpmcLockfreeQueue`                 |          | Multi producer, multi consumer lock-free FiFo with ringbuffer like overflow handling                                                                                                                                               |
|`MpmcLoFFLi`                        | i        | Lock-free LIFO based index manager (lock-free free list). One building block of our memory manager. After construction it contains the indices {0 ... n} which you can acquire and release.                                        |
|`SpscSofi`                          | i        | Single producer, single consumer lock-free safely overflowing FiFo (SpscSofi).                                                                                                                                                         |
|`MpmcResizeableLockFreeQueue`       |          | Resizeable variant of the `MpmcLockfreeQueue`                                                                                                                                                                                          |
|`stack`                             |          | Stack implementation with simple push/pop interface.                                                                                                                                                                               |

#### Attribute overview of the available buffers

| Data Structure           | Shared Memory usable  | Thread-Safe | Lock-Free | Concurrent Producers : Consumers | Bounded Capacity | Data Type Restriction | Use Case                                                                                                                |
|--------------------------|-----------------------|-------------|-----------|----------------------------------|------------------|-----------------------|-------------------------------------------------------------------------------------------------------------------------|
|`SpscFifo`                | Yes                   | Yes         | Yes       | 1:1                              | Yes              | Copyable              | FIFO Data transfer                                                                                                      |
|`MpmcLockfreeQueue`       | Yes                   | Yes         | Yes       | n:m                              | Yes              | Copyable or Movable   | lock-free transfer of arbitrary data between multiple contexts in FIFO order with overflow handling (ringbuffer)        |
|`MpmcLoFFLi`              | Yes                   | Yes         | Yes       | n:m                              | Yes              | int32                 | manage memory access, LIFO order                                                                                        |
|`SpscSofi`                | Yes                   | Yes         | Yes       | 1:1                              | Yes              | Trivially Copyable    | lock-free transfer of small data (e.g. pointers) between two contexts in FIFO order with overflow handling (ringbuffer) |
|`MpmcResizeableLockFreeQueue`| Yes                   | Yes         | Yes       | n:m                              | Yes              | Copyable or Movable   | Resizeable variant of the `MpmcLockfreeQueue`                                                                         |
|`stack`                   | Yes                   | No          | -         | -                                | Yes              | None                  | Stack for a single-threaded application                                                                                 |

### Inter-process communication (ipc)

| class                              | internal | description                                                                                                                                                                                                              |
|:----------------------------------:|:--------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`UnixDomainSocket`                  |          | Interface for unix domain sockets.                                                                                                                                                                                       |
|`MessageQueue`         | Interface for Message Queues, see [ManPage mq_overview](https://www.man7.org/linux/man-pages/man7/mq_overview.7.html).                                                                                                                                                                                                |
|`NamedPipe`            | Shared memory based IPC channel. Mainly a `UnixDomainSocket` replacement on Windows.                                                                                                                                                                                                                                  |
|`PosixSharedMemoryObject`           |          | Creates and maps existing shared memory into the application.                                                                                                                                                            |
|`PosixMemoryMap`                    | i        | Abstraction of `mmap`, `munmap` and helper class for the `PosixSharedMemoryObject`.                                                                                                                                           |
|`PosixSharedMemory`                 | i        | Abstraction of shared memory, see [ManPage shm_overview](https://www.man7.org/linux/man-pages/man7/shm_overview.7.html) and helper class for the `PosixSharedMemoryObject`.                                                   |

### Threads & sychronisation (sync)

| class                 | internal | description                                                                                                                                                                                                                           |
|:---------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`PeriodicTask`         | i        | Periodically executes a callable specified by the template parameter in a configurable time interval.                                                                                                                                 |
|`smart_lock`           |          | Creates arbitrary thread-safe constructs which then can be used like smart pointers. If some STL type should be thread safe use the smart_lock to create the thread safe version in one line. Based on some ideas presented in [Wrapping C++ Member Function Calls](https://stroustrup.com/wrapper.pdf) |
|`mutex`                | i        | Mutex interface, see [ManPage pthread_mutex_lock](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html).                                                                                                                  |
|`UnnamedSemaphore`     |          | Unamed semaphore interface, see [ManPage sem_overview](https://man7.org/linux/man-pages/man7/sem_overview.7.html)                                                                                                                     |
|`NamedSemaphore`       |          | Named semaphore interface, see [ManPage sem_overview](https://man7.org/linux/man-pages/man7/sem_overview.7.html)                                                                                                                      |
|`thread`               |          | Heap-less replacement for `std::thread`.                                                                                                                                                                                              |
|`SignalGuard`          |          | Helper class for signal handler registration.                                                                                                                                                                                         |
|`SignalWatcher`        |          | Batteries included signal handling with polling and optional blocking wait for `SIGINT` and `SIGTERM`.                                                                                                                                                                                                                |

### Generalized design patterns & abstractions (design)

| class                 | internal | description                                                         |
|:---------------------:|:--------:|:--------------------------------------------------------------------|
|`Builder`              |          | Macro which generates a setter method useful for a builder pattern. |
|`IOX_POSIX_CALL`       |          | Wrapper around C and POSIX function calls which performs a full error handling. Additionally, this wrapper makes sure that `EINTR` handling is performed correctly by repeating the system call. |
|`functional_interface` |          | Constructs to easily add functional interfaces like `and_then` to object container.                                                                                                                                                   |
|`NewType<T, Policies>` |          | Implementation of [Haskells NewType-pattern](https://wiki.haskell.org/Newtype).                                                                                                                                                 |
|`StaticLifetimeGuard`  |          | Static instance manager which solves the singleton lifetime problem. |
|`PolymorphicHandler`   |          | Singleton handler with a default instance that can be changed at runtime. |

### Reporting (reporting)

The error handler is a central instance for collecting all errors and react to them. The `error-handling.hpp` contains a list of all error enum values. The error handler has different error levels, for more information see [error-handling.md](../doc/design/error-handling.md)
For information about how to use the logger API see [error-handling.md](../doc/design/error-handling.md).

| class                   | internal | description                                                                                                             |
|:-----------------------:|:--------:|:------------------------------------------------------------------------------------------------------------------------|
|`errorHandler`           |          | Free function to call the error handler with a defined error and an error level, see header file for practical example. |
|`ErrorHandler`           | i        | error handler class only for testing purposes, should not be used directly                                              |
|`logger`                 |          |                                                                                                                         |
|`requires`               |          | Base for `IOX_EXPECTS`/`IOX_ENSURES` from the C++ Core Guideline                                                                |

### Time (time)

Never use physical properties like speed or time directly as integer or float in your code.
Otherwise you encounter problems like this function `void setTimeout(int timeout)`. What is the unit of the argument, seconds? minutes? If you use `Duration` you see it directly in the code.

```cpp
void setTimeout(const Duration & timeout);

setTimeout(11_s); // 11 seconds
setTimeout(5_ms); // 5 milliseconds
```

| class               | internal | description                                                                                                                                 |
|:-------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------|
|`Duration`           | i        | Represents the unit time, is convertible to `timespec` and `timeval`. User defined literals are available for convenience and readability.  |
|`deadline_timer      |          | Polling based timer to check for an elapsed deadline.                                                                                       |
|`adaptive_wait`      | i        | Building block to realize busy waiting loops with low CPU load.                                                                             |

### Authentication (auth)

| class                 | internal | description                                                                                                                                                                                                                           |
|:---------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`posix_group`          |          | Access to user information.                                                                                                                                                                                                 |
|`posix_user`           |          | Access to group information.                                                                                                                                                                                                 |


### Comand line interface (cli)

| component             | internal | description                                                                                                                                                                                                                           |
|:---------------------:|:--------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|`cli_definition`       |          | Classes and macros to create command line interfaces.                                                                                                                                                                                                 |

<center>
[Check out iceoryx_hoofs on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_hoofs/){ .md-button } <!--NOLINT required only for the website, github URL required-->
</center>
