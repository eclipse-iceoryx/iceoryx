# iceoryx on 32 bit architectures

iceoryx works on 32-bit hardware, but only as technology preview and is not meant for production.

See also [here](https://github.com/eclipse-iceoryx/iceoryx/issues/2301) for more details and the limitations sections in this document.

## Dependencies

For 32-bit support, the following packages need to be installed on ubuntu

```bash
sudo dpkg --add-architecture i386
sudo apt install libacl1-dev:i386 libc6-dev-i386 libc6-dev-i386-cross libstdc++6-i386-cross gcc-multilib g++-multilib
```

## iceoryx as 32-bit library

## Build steps

The simplest way to build iceoryx is via the `iceoryx_build_test.sh` script

```bash
tools/iceoryx_build_test.sh release 32-bit-x86
```

If the script cannot be used, this are the steps with `cmake` on x86

```bash
cmake -S iceoryx_meta -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-m32 -malign-double" -DCMAKE_CXX_FLAGS="-m32 -malign-double"
cmake --build build
```

The `-m32` flag tells GCC to build iceoryx as 32-bit library on a 64-bit system.
The `-malign-double` flag is required to have 64-bit atomics on an 8 byte boundary.
Furthermore, it is required for the 32-64 bit mix-mode to enforce the same data layout when 32-bit application communicate with 64-bit applications.

> [!NOTE]
> On Windows with MSVC, the `-DCMAKE_GENERATOR_PLATFORM=Win32` cmake flag must be set instead of the `-DCMAKE_C_FLAGS` and `-DCMAKE_CXX_FLAGS`.

## Limitations

An internal data structure, the `UsedChunkList`, might be left in a corrupt state when an application terminates abnormally when writing to this data structure.
In order to detect torn-writes on 32-bit, the data structure needs to be refactored.

## iceoryx for communication between 32-bit and 64-bit applications aka 32-64 bit mix-mode

## Attention

Mixing 32-bit and 64-bit applications in a shared-memory environment is a non-trivial endeavor.
Since the data structures are shared between applications with varying bitness, one has to take special care of the layout of the data structures shared between the applications.

For example, the following struct has a size of 16 bytes and is aligned to 8 byte on common 64-bit architectures like x86-64.
But on common 32-bit architectures like x86, it has a size of 12 bytes and is aligned to 4 bytes.

```cpp
struct Foo {
    bool bar {false};
    uint64_t baz {0};
};
```

As long as the applications share the same bitness, there is no need for special consideration.
However, when connecting 32-bit and 64-bit applications via shared memory, both must adhere to a common memory layout.
If the layout differs, it can lead to unpredictable behavior and errors in the applications.

The simplest way to fix this specific alignment issue, is to use the `-malign-double` flag, which enforces an 8 byte alignment boundary for 64-bit data types on 32-bit architectures.

### Build steps

Similar to the 32-bit build, the simplest way to build for the 32-64 bit mix-mode is the `iceoryx_build_test.sh` script

```bash
tools/iceoryx_build_test.sh release examples 32-bit-x86 experimental-32-64-bit-mix-mode --build-dir build-32
tools/iceoryx_build_test.sh release examples experimental-32-64-bit-mix-mode --build-dir build-64
```

If the script cannot be used, this are the steps with `cmake` on x86

```bash
cmake -S iceoryx_meta -B build-32 -DCMAKE_BUILD_TYPE=Release -DEXAMPLES=ON -DCMAKE_C_FLAGS="-m32 -malign-double" -DCMAKE_CXX_FLAGS="-m32 -malign-double" -DIOX_EXPERIMENTAL_32_64_BIT_MIX_MODE=ON
cmake --build build-32

cmake -S iceoryx_meta -B build-64 -DCMAKE_BUILD_TYPE=Release -DEXAMPLES=ON -DIOX_EXPERIMENTAL_32_64_BIT_MIX_MODE=ON
cmake --build build-64
```

> [!NOTE]
> On Windows with MSVC, there is now counterpart for `-malign-double` and therefore the 32-64 bit mix-mode does not yet work on Windows.

## Running the examples

You can now mix and match 32-bit and 64-bit applications

```bash
# terminal 1
build-32/iox-roudi

# terminal 2
build-64/iceoryx_examples/request_response/iox-cpp-request-response-listener-server

# terminal 3
build-32/iceoryx_examples/request_response/iox-cpp-request-response-waitset-client
```

### Limitations

In addition to the limitations of the 32-bit iceoryx, the mix-mode needs to ensure that all the data structures in shared memory have the same layout.
While the `-malign-double` flag can be used for the iceoryx data types, it does not work for POSIX data structures like `sem_t`.
These data types also have a different size for 32-bit and 64-bit architecture and are used in iceoryx in the shared-memory, e.g. for the `WaitSet`.
In order to make the iceoryx applications interoperable between 32-bit and 64-bit, a spin lock and a spin semaphore is used for their POSIX counterparts.
This can increase the CPU load and also the latency.

For a production environment, the spin semaphore and spin lock needs to be replaced by a `futex` on Linux and a `WaitOnAddress` call on Windows.
For other OSes, a proper solution is yet to be found.
