# automotive_soa

## Introduction

>
> Attention!
>
> The example is not a production-ready `ara::com` binding.
>

This example gives a brief overview on how to use iceoryx as an [AUTOSAR Adaptive](https://www.autosar.org/standards/adaptive-platform/)
[`ara::com`](https://www.autosar.org//fileadmin/user_upload/standards/adaptive/20-11/AUTOSAR_EXP_ARAComAPI.pdf) binding.
It covers only a small amount of the overall AUTOSAR Adaptive functionality and takes some shortcuts.
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

* Implemented as singleton
* Initalizes `runtime::PoshRuntime` which registers with RouDi and sets up shared memory (see overview.md)
* Uses `popo::Listener` and `runtime::ServiceDiscovery` to provide service discovery
* `FindService` searches for any service kind publish/ subscribe and request/response
* `StartFindService` uses `Listener` to wakeup if the service registry changed

### Minimal skeleton

* Contains all three communication patterns from `ara::com`
  * Event
  * Field
  * Method

* Unlike in AUTOSAR, in iceoryx a service is represented by the individual `Publisher` or `Server`
* AUTOSAR service is considered as available as soon as all members are available

#### `EventPublisher`

#### `FieldPublisher`

#### `MethodServer`

### Skeleton `main()`

* Create runtime
* Create `MinimalSkeleton`
* Event
  * Increasing counter
  * Used to measure latency
  * Topics with different sizes are available
  * Also available as UNIX domain socket implementation
* Field
  * Sends inital value on creation
  * After 30 loop iteration the value is updated
* Method
  * Works in the background, in its own thread
  * Receives requests, computes answer and sends back response

### Minimal proxy

* Contains the respective counterparts to be able to consume the
`MinimalSkeleton` service with all its three elements aka iceoryx services

#### `EventSubscriber`

#### `FieldSubscriber`

#### `MethodClient`

### Proxy `main()`

* Discovery phase, two kinds
  * Synchronous `FindService` call
  * If Skeleton app was not startet yet, asynchronous find service call is set up
  * Once `MinimalSkeleton` instance becomes available a callback will be executed, which will create the `MinimalProxy` object
* The main() is then able to run through its routine
  * Field is received every second
  * Remote-procedure call `computeSum()` is executed every second
  * Receiving the data on the event is done in an asynchronous manner by setting up a receiver handler, which is callted instantly once data has arrived

<center>
[Check out automotive_soa on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/automotive_soa){ .md-button } <!--NOLINT github url for website-->
</center>
