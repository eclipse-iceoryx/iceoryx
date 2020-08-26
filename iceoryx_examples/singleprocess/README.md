# singleprocess - Transfer data between threads, all in one process

## Introduction

This example demonstrates how iceoryx can be used in a single process for
inter thread communication. This is for instance helpful if you would like
to establish a simple communication API in a 3D engine with a multitude of
threads which are interacting without starting separately RouDi everytime.

## Run singleprocess

The example can be started with
```sh
./build/iceoryx_examples/singleprocess/single_process
```

After you have started the example you should see an output like
```
Log level set to: [ Error ]
Reserving 71546016 bytes in the shared memory [/iceoryx_mgmt]
[ Reserving shared memory successful ]
Reserving 149655680 bytes in the shared memory [/users]
[ Reserving shared memory successful ]
RouDi is ready for clients
Sending: 0
Sending: 1
Receiving : 1
Sending: 2
Receiving : 2

...
```

The first lines until `RouDi is ready for clients` are coming from the RouDi
startup in which the shared memory management segment and user data segment are
created.

Afterwards the sender and receiver thread are started and are beginning to
transmit and receive data.

## Code Walkthrough

### Creating a Single Process RouDi, Sender and Receiver

 1. We start by setting the log level to error since we do not want to see all the
    debug messages.

    ```cpp
    int main()
    {
        // set the log level to error to see the essence of the example
        iox::log::LogManager::GetLogManager().SetDefaultLogLevel(iox::log::LogLevel::kError);
    ```

 2. To start RouDi we have to create a configuration for him. We are choosing the
    default config. Additionally, RouDi needs some other components like a memory
    management unit which handles how the memory is created in which the transmission
    data is stored. The `IceOryxRouDiComponents` class is handling them for us
    ```cpp
    iox::RouDiConfig_t defaultRouDiConfig = iox::RouDiConfig_t().setDefaults();
    iox::roudi::IceOryxRouDiComponents roudiComponents(defaultRouDiConfig);
    ```

 3. We are starting RouDi and provide him with the required components. Furthermore, we
    disable monitoring. The last bool parameter `false` states that RouDi does not
    terminate all registered processes when he goes out of scope. If we would set it
    to `true`, our application would self terminate when the destructor is called.
    ```cpp
    iox::roudi::RouDi roudi(
        roudiComponents.m_rouDiMemoryManager, roudiComponents.m_portManager, iox::roudi::MonitoringMode::OFF, false);
    ```

 4. Here comes a key difference to an inter process application. If you would like
    to communicate within one process you have to use `PoshRuntimeSingleProcess`.
    You can create only one at a time!
    ```cpp
    iox::runtime::PoshRuntimeSingleProcess runtime("/singleProcessDemo");
    ```

 5. Now that everything is up and running we can start the sender and receiver
    thread, wait two seconds and then stop the example.
    ```cpp
    std::thread receiverThread(receiver), senderThread(sender);

    // communicate for 2 seconds and then stop the example
    std::this_thread::sleep_for(std::chrono::seconds(2));
    keepRunning.store(false);

    senderThread.join();
    receiverThread.join();
    ```

### Implementation of Sender and Receiver
Since there are no differences to inter process ports you can take a look at the
[icedelivery example](../icedelivery) for a detailled documentation. We only provide
you here with a short overview.

#### Sender
We create a publisher port with the following service description
(Service = `Single`, Instance = `Process`, Event = `Demo`) and offer our service
to the world.
```cpp
iox::popo::Publisher publisher({"Single", "Process", "Demo"});
publisher.offer();
```
After that we are sending
incrementing numbers with an 100ms interval in a `while` loop till the
variable `keepRunning` is false.
```cpp
while (keepRunning.load())
{
    auto sample = static_cast<TransmissionData_t*>(publisher.allocateChunk(sizeof(TransmissionData_t)));
    sample->counter = counter++;
    consoleOutput(std::string("Sending: " + std::to_string(sample->counter)));
    publisher.sendChunk(sample);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

#### Receiver
Like with the sender we are creating a corresponding subscriber port with the
same service description and subscribe to our service.
```cpp
iox::popo::Subscriber subscriber({"Single", "Process", "Demo"});

uint64_t cacheQueueSize = 10;
subscriber.subscribe(cacheQueueSize);
```
Now we can receive the data in a while loop till `keepRunning` is false. But we
only try to acquire data if our `SubscriptionState` is `SUBSCRIBED`.
```cpp
while (keepRunning.load())
{
    if (iox::popo::SubscriptionState::SUBSCRIBED == subscriber.getSubscriptionState())
    {
        const void* rawSample = nullptr;
        while (subscriber.getChunk(&rawSample))
        {
            auto sample = static_cast<const TransmissionData_t*>(rawSample);
            consoleOutput(std::string("Receiving : " + std::to_string(sample->counter)));
            subscriber.releaseChunk(rawSample);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```
