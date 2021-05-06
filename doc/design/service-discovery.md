# Service Discovery

## Summary and problem description

Service discovery over IPC channel e.g. message queue or UNIX domain socket is not performant since larger data is transferred, which can lead to transmission of several frames. If lots of services are discovered at high-frequency e.g. at startup the IPC channel can become a bottleneck.

### Status quo of iceoryx Almond

#### Runtime

* `PoshRuntime::findService(capro::ServiceDescription)`
  * Sends message over IPC channel

#### RouDi

* `PortManager::findService(capro::ServiceDescription)`
  * Called after RouDi has received a request from the runtime

## Terminology

## Design

Requirements:

* Polling and non-polling aproach shall both be possible

### Considerations

#### AUTOSAR Adaptive

* findService() call as implemented in Almond v1.0
* ara::com service discovery callbacks

1. `StartFindService()`: Register callback, std::function interface
1. `StopFindService()`: Unregister callback
1. `FindService()`: one-shot, polling, synchronous

#### SOME/IP-SD

@todo ask Marika about how the API looks

#### DDS/ROS2/CycloneDDS

### Alternative A

* Id's + ABA counter over IPC channel
* Data transfer over shared memory

Pro:

* Fast data transfer

Contra:

* More complex than todays solution

### Alternative B

* Built-in topic
  * What should be contained inside the topic? This needs to be specified

Pro:

* Close to DDS mechanism

Con:

* Lots of memory needed, dimensioning according to max values

### Alternative C

* Combination of A & B
  * What will RouDi do if he runs out of memory in B?

### Solution

### Code example

## Open issues

* What to do with `getServiceRegistryChangeCounter()` remove?
* How does the ara::com service discovery API look like?
* How does the DDS service discovery API look like?
* How does the SOME/IP-SD service discovery API look like?
* Try out a `ros topic list`
