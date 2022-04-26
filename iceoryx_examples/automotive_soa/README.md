# automotive_soa

## Introduction

>
> Attention!
>
> The example is not a production-ready `ara::com` binding.
>

This example gives a brief overview on how to use iceoryx as an [AUTOSAR Adaptive](https://www.autosar.org/standards/adaptive-platform/)
[`ara::com`](https://www.autosar.org//fileadmin/user_upload/standards/adaptive/20-11/AUTOSAR_EXP_ARAComAPI.pdf) binding.
It covers only a small amount of the overall AUTOSAR Adaptive functionality.
If a complete binding is needed, please [contact the AUTOSAR foundation](https://www.autosar.org/how-to-join/) and become a member.

The example shows the three different ways of communication between a skeleton and a proxy:

1. Publish/ subscribe communication with events
1. Accessing a value with fields
1. Calling a remote method

## Expected Output

<!-- [![asciicast](https://asciinema.org/a/000000.svg)](https://asciinema.org/a/000000) -->

## Code walkthrough

`MinimalProxy` and `MinimalSkeleton` are typically generated.

### Runtime

### Minimal skeleton

#### `EventPublisher`

#### `FieldPublisher`

#### `MethodServer`

### Skeleton `main()`

### Minimal proxy

#### `EventSubscriber`

#### `FieldSubscriber`

#### `MethodClient`

### Proxy `main()`

<center>
[Check out automotive_soa on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/automotive_soa){ .md-button } <!--NOLINT github url for website-->
</center>
