# Service Discovery

## Summary and problem description

Service discovery over IPC channel e.g. message queue or UNIX domain socket is not performant since large data is transferred and something at high-frequency it can become a bottleneck.

## Terminology

## Design

Requirements:

* Polling and non-polling aproach

### Considerations

* AUTOSAR Adaptive
  * findService() call as implemented in Almond v1.0
  * ara::com service discovery callbacks
* SOME/IP-SD
* DDS/ROS2


#### Alternative A

* Id's + ABA counter over IPC channel
* Data transfer over shared memory

#### Alternative B

* Built-in topic

Pro:
* Close to DDS mechanism

Con:
* Lots of memory needed, dimensioning according to max values

#### Alternative C

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
