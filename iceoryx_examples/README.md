## Howto build the examples

To build the examples you can use the cmake configuration which is provided
in `iceoryx_meta`.
```sh 
cmake -Bbuild -Hiceoryx_meta -Dexamples=ON # by default the examples are always build
cd build 
make
```

To run an example switch into the `iceoryx_examples` directory which can be 
found in our build directory. Then select your example and follow the instructions
from the examples readme.

```sh 
cd build/iceoryx_examples/
cd someExample
./runExampleCode
```

## List of all examples

| example                                                | description |
|:-------------------------------------------------------|:------------|
|[benchmark_optional_and_expected](./benchmark_optional_and_expected/)        | Benchmark of optional and expected in a collection of use cases which can be found in iceoryx. |
|[icecrystal](./icecrystal/)                             | Demostrates the usage of the iceoryx introspection client. |
|[icedelivery](./icedelivery/)                           | You are new to iceoryx then take a look at this example which demonstrates the basics of iceoryx by sending data from one process to another process. |
|[icedelivery_on_c](./icedelivery_on_c/)                 | Shows the same use case as the ice delivery example but with the iceoryx C API  |
|[iceperf](./iceperf/)                                   | A benchmark application which measures the latency of an IPC transmission between two applications. |
|[iceperf](./iceperf/)                                   | A benchmark application which measures the latency of an IPC transmission between two applications. |
|[singleprocess](./singleprocess/)                       | Iceoryx can also be used for inter thread communication when you would like to run everything in a single process. |
