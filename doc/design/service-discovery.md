# Service Discovery

## Summary and problem description

Service discovery over IPC channel e.g. message queue or UNIX domain socket is not performant since larger data is transferred, which can lead to transmission of several frames. If lots of services are discovered at high-frequency e.g. at startup the IPC channel can become a bottleneck.

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

### Considerations

#### AUTOSAR Adaptive

* findService() call as implemented in Almond v1.0
* ara::com service discovery callbacks

1. `StartFindService()`: Register callback, std::function interface
1. `StopFindService()`: Unregister callback
1. `FindService()`: one-shot, polling, synchronous

#### SOME/IP-SD

* Very similar to AUTOSAR Adaptive
* @todo ask Marika about the details

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

### Alternative A

* Id's + ABA counter over IPC channel
* Data transfer over shared memory
  * Put Service Registry to shared memory so everyone can access it?

Pro:

* Fast data transfer

Contra:

* More complex than todays solution

### Alternative B

* Built-in topic
  * What should be contained inside the topic? This needs to be specified
  * What will RouDi do if he runs out of memory in B?

Pro:

* Close to DDS mechanism

Con:

* Dimensioning according to max values is not optimal
* Presumably lots of memory needed, e.g. during startup phase when lots of apps will do discovery

### Alternative C

* Combination of A & B?

### Alternative D

* Create a thread in `PoshRuntime`, which polls for specific discovery information

### Solution

### Code example

Possible bindings with built-in topic:

#### Event-based notification

```cpp
//Subscriber<DiscoveryTopic> discoverySubscriber;

auto& runtime = PoshRuntime::initRuntime("myApp");
auto* interfacePortData = runtime.getMiddlewareInterface(capro::Interfaces::INTERNAL);

InterfacePort caproSubscriber(interfacePortData);
Listener myListner;

void onDiscoveryUpdateCallback(InterfacePort* subscriber)
{
    //subscriber->take().and_then([](){
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

#### Polling

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
* [ ] How does the SOME/IP-SD service discovery API look like?
* [ ] Try out a `ros topic list`
