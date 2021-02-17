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

| Example                                                | Description |Level |
|:-------------------------------------------------------|:------------|:----:|
|[callbacks](./callbacks/)                           | Implement callbacks which are triggered by events. | Intermediate |
|[icedelivery](./icedelivery/)                           | You are new to iceoryx then take a look at this example which demonstrates the basics of iceoryx by sending data from one process to another process. | Beginner |
|[icedelivery_in_c](./icedelivery_in_c/)                 | Shows the same use case as the ice delivery example but with the iceoryx C API  | Beginner |
|[ice_multi_publisher](./ice_multi_publisher/)           | Shows how multiple publishers can be used to publish on the same topic. | Intermediate |
|[waitset](./waitset/)                       | Explaining the structure, usage and ideas behind the WaitSet. | Intermediate |
|[waitset_in_c](./waitset_in_c/)                 | The C example of our WaitSet. | Intermediate |
|[singleprocess](./singleprocess/)                       | Iceoryx can also be used for inter thread communication when you would like to run everything in a single process. | Intermediate |
|[iceperf](./iceperf/)                                   | A benchmark application which measures the latency of an IPC transmission between two applications. | Advanced |
|[icecrystal](./icecrystal/)                             | Demonstrates the usage of the iceoryx introspection client. | Advanced |
