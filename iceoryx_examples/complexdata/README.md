# complexdata

## Introduction

To implement zero-copy data transfer we use a shared memory approach. This requires that every data structure needs to be entirely
contained in the shared memory and must not internally use pointers or references. The complete list of restrictions can be found
[here](https://iceoryx.io/latest/getting-started/overview/#restrictions). Therefore, most of the STL types cannot be used, but we
reimplemented some [constructs](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_utils#cxx). This example shows how
to send/receive a iox::cxx::vector and how to send/receive a complex data structure containing some of our STL container surrogates.

<!--## Expected Output-->

<!-- @todo Add expected output with asciinema recording before v2.0-->
<!-- @todo multiple examples described in here, expected output should be in front of every example -->

## Code Walkthrough

introduction

### Publisher application sending a `iox::cxx::vector`

In this example we want our publisher to send a vector containing double. Since we cannot use dynamic memory, we use the 
`iox::cxx::vector` with a capacity of 5.

```cpp
iox::popo::Publisher<iox::cxx::vector<double, 5>> publisher({"Radar", "FrontRight", "VectorData"});
```

We use a while-loop similar to the one described in the 
[icedelivery example](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/icedelivery) to send the
vector to the subscriber. After successfully loaning memory we append elements to the vector until it's full.

```cpp
for (uint64_t i = 0U; i < sample->capacity(); ++i)
{
    sample->emplace_back(static_cast<double>(ct + i));
}
```

The only difference here to the `std::vector` is that `emplace_back` returns a bool - true if the appending was successful,
false otherwise. `emplace_back` fails when the vector is already full. In our case, we can omit the check of the return value
since the for-loop doesn't exceed the capacity of the vector.

### Subscriber application receiving a `iox::cxx::vector`

Our subscriber application iterates over the received vector to print its entries to the console. Note that the `separator` is only
used for a easy to read output.

```cpp
for (const auto& entry : *sample)
{
    s << separator << entry;
}
std::cout << s.str();
```

### Publisher application sending a complex data structure

### Subscriber application receiving a complex data structure

<center>
[Check out complexdata on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/complexdata){ .md-button }
</center>
