# Service Discovery

## Summary and problem description

Service discovery over IPC channel e.g. message queue or UNIX domain socket is not performant since larger data is
transferred, which can lead to transmission of several frames. If lots of services are discovered at high-frequency
e.g. at startup the IPC channel can become a bottleneck. Furthermore, a lession learned from the iceoryx development
so far is, that the IPC channel should just be used for the earliest communication during startup and not for the
creation of objects inside the shared memory (see [this issue](https://github.com/eclipse-iceoryx/iceoryx/issues/611)).

### Status quo in iceoryx Almond

#### Runtime

* `cxx::expected<InstanceContainer, FindServiceError> PoshRuntime::findService(capro::ServiceDescription)`
  * Sends message over IPC channel

#### RouDi

* `runtime::IpcMessage {ProcessManager,PortManager}::findService(capro::ServiceDescription)`
  * Called after RouDi has received a request from the runtime
  * Sends back answer to `PoshRuntime` over IPC channel

## Terminology

## Requirements

* Polling and event notification approach shall both be possible

### Considerations and use-cases

#### AUTOSAR Adaptive

* findService() call as implemented in Almond v1.0
* ara::com service discovery callbacks

1. `StartFindService()`: Register callback, std::function interface
1. `StopFindService()`: Unregister callback
1. `FindService()`: one-shot, polling, synchronous

#### SOME/IP-SD

* Very similar to AUTOSAR Adaptive

#### Cyclone DDS and ROS 2

* Cyclone DDS calls this feature [topic discovery](https://github.com/eclipse-cyclonedds/cyclonedds/blob/master/docs/dev/topic_discovery.md). See also the [feature request](https://github.com/eclipse-cyclonedds/cyclonedds/issues/42).

* Based on SDEP (simple endpoint discovery protocol)
* `dds_ktopic`
  * stores pointer to `ddsi_topic_definition`
* Has built-in DCPSTopic topic and contains
  * Topic name
  * Type name
  * Topic key
  * QoS settings
* `dds_find_topic_locally` function is available for users

#### Static discovery

* Static discovery shall be supported
  * Having RouDi read in a config file, which has
    * All the topics listed
    * Wiring pub/sub information
  * During init phase pub/sub are connected
* Create abstract interface for `ServiceRegistry`
  * Add new class to `StaticServiceRegistry::StaticServiceRegistry(std::map<CaproIdString_t, instance_t> map)`
    * No notification-based callback possibilty
    * Just static, polling with `FindService()` possible

#### Sychronous, one-shot RPC (Polling)

##### Alternative A: Request/ response feature usage

* Re-use [request/ response feature](https://github.com/eclipse-iceoryx/iceoryx/issues/27)

Pro:

* Re-use building blocks to avoid code duplication
* Dogfooding
* Fast due to shared memory

Con:

* Shared memory approach might have overhead compared for a simple RPC call

##### Alternative B: IPC channel usage

Stick with IPC channel

Pro:

* No code changes needed

Con:

* No userspace solution
  * IPC channel uses OS functions
* Upper frame limit may lead to segregation into several frames

#### Event-based notification

##### Alternative A: Shared memory + IPC channel

* Send ID's + ABA-counter/ServiceDiscoveryChangeCounter over IPC channel
* Data transfer over shared memory
  * Basically putting Service Registry to shared memory so everyone can access it

Pro:

* Fast data transfer

Contra:

* More complex than todays solution
  * ABA counter needed

##### Alternative B: Built-in topic

* Built-in topic approach based on `InterfacePort`'s
  * Sniff and intercept `CaproMessage`
* Also see
  * [overview diagram](diagrams/service-discovery-sequence-diagram.puml)
  * [sequence diagram](diagrams/service-discovery-overview.puml)

Pro:

* Close to DDS mechanism
* `InterfacePort` already available
* Synergies with gateway mechanisms
* Gateways can benefit from event notification mechanism
  * No need for a polling-loop anymore

Con:

* Overhead, `Listner` will wake up on every `CaproMessage`
* What will RouDi do if he runs out of memory?
  * Dimensioning according to max values is not optimal (MAX_INTERFACE_CAPRO_FIFO_SIZE)
  * Presumably lots of memory needed, e.g. during startup phase when lots of apps will do discovery
  * A safety-certified middleware based on iceoryx would not use the dynamic discovery feature
Remark:

* Need to filter out your own `CaproMessageType::FIND` requests
* Extend `dispatchCaproMessage()` with condition variable and notification mechanism

##### Alternative C: Custom thread

* Create a custom thread in `PoshRuntime`, which polls for specific discovery information using request/response

Pro:

* More flexible
* Re-use building blocks

Con:

* Not needed by all users per default
* Polling leads to overhead

### Decision

* Sychronous, one-shot RPC (Polling): Alternative A
* Event-based notification: Alternative B

### Code example

Possible bindings with alternative B:

#### Event-based notification

```cpp
auto& runtime = PoshRuntime::initRuntime("myApp");
auto* interfacePortData = runtime.getMiddlewareInterface(capro::Interfaces::INTERNAL);

InterfacePort caproSubscriber(interfacePortData);
Listener myListner;

void onDiscoveryUpdateCallback(InterfacePort* subscriber)
{
    subscriber->tryGetCaProMessage().and_then([](auto& caproMessage){
        // Has any service that is relevant for me changed?
        if(caproMessage.m_serviceDescription == myRelevantServiceDescription && someOtherCondition())
        {
            // call user-defined callback
        }
    });
}

myListner.attachEvent(caproSubscriber, DATA_RECEIVED, createNotificationCallback(onDiscoveryUpdateCallback));
```

#### Sychronous, one-shot RPC (Polling)

```cpp
// Binding code
auto& runtime = PoshRuntime::initRuntime("myApp");

runtime.findService({"Radar", "FrontLeft", "Objects"}).and_then([&](auto& container){
    // search the container
});


// Runtime implementation
cxx::expected<InstanceContainer, FindServiceError>
PoshRuntime::findService(const capro::ServiceDescription& serviceDescription) noexcept
{
    // Needs to be sychronous call, use request response feature here
    auto findServiceRequest = m_client.loanRequest();
    m_client.sendRequest(findServiceRequest, serviceDescription);

    m_client.takeResponse().and_then([&](){
        // fill the instance container
    });
    return instanceContainer;
}
```

## Open issues

* [x] What to do with `getServiceRegistryChangeCounter()` remove? Depends could be useful as ABA counter
* [x] How does the ara::com service discovery API look like?
* [x] How does the DDS service discovery API look like?
* [x] How does the SOME/IP-SD service discovery API look like?
* [ ] What does a `ros topic list` do in rmw_iceoryx?
* [ ] Filter for `ServiceDescription::EventString` needed by AUTOSAR? Not supported by `ServiceRegistry::find()` as of today
* [ ] Decision on mapping table between iceory and DDS (see [overview.md](../website/getting-started/overview.md))
  * [ ] Current mapping table between iceoryx and DDS does not work with service discovery
