# QoS policies

Unlike other middlewares, e.g. [DDS used in ROS 2](https://docs.ros.org/en/galactic/Concepts/About-Quality-of-Service-Settings.html),
iceoryx keeps things simple and offers basic Quality of Service (QoS) settings. The QoS
settings are called options and can be used to optimize and tailor the communication.

## `ServerOptions` and `ClientOptions`

The two most important settings are:

| Option                                 | Explanation                                                  |
|----------------------------------------|--------------------------------------------------------------|
| `ServerOptions::requestQueueCapacity`  | This enables servers to store n requests at the users side   |
| `ClientOptions::responseQueueCapacity` | This enables clients to store n responses at the users side  |

### Compatible policies

The `ConsumerTooSlowPolicy` and `QueueFullPolicy` are used to control the behavior for full queues. Since some
combinations would be contradicting, not all of them match to connect producer and consumer.

| `ConsumerTooSlowPolicy`   | `QueueFullPolicy`     | Behavior                                   | Connection          |
|---------------------------|-----------------------|--------------------------------------------|---------------------|
| `WAIT_FOR_CONSUMER`       | `BLOCK_PRODUCER`      | Producer blocks and waits for consumers    | :white_check_mark:  |
| `WAIT_FOR_CONSUMER`       | `DISCARD_OLDEST_DATA` | Non-blocking producer                      | :white_check_mark:  |
| `DISCARD_OLDEST_DATA`     | `DISCARD_OLDEST_DATA` | Non-blocking producer                      | :white_check_mark:  |
| `DISCARD_OLDEST_DATA`     | `BLOCK_PRODUCER`      | Not compatible, no connection established  | :x:                 |

## Server and client matching criteria

Two criteria have to be fulfilled in order for a server and a client to be connected.

1. Same `capro::ServiceDescription`
2. Matching `ConsumerTooSlowPolicy` and `QueueFullPolicy` in `ServerOptions`/`ClientOptions`

## `PublisherOptions` and `SubscriberOptions`

The three most important settings are:

| Option                              | Explanation                                                                                                                                                   |
|-------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `PublisherOptions::historyCapacity` | This enables late-joining subscribers to request the n last samples                                                                                           |
| `SubscriberOptions::queueCapacity`  | Size of the subscriber queue where samples are stored at the user side                                                                                        |
| `SubscriberOptions::historyRequest` | The number of samples a late-joining subscriber will request from every offered publisher, should be equal or smaller than `historyCapacity` otherwise capped |

!!! warning
    In case of n:m communication, the history feature will **not** provide the overall last n samples based on delivery point in time!

    The following two scenarios are examples of issues when using n:m together with the history feature:

    1. Multiple publishers
    The last n samples of every publisher are received. This means for m publishers in the worst
    case m * n samples in random order, not in the order they were published on the topic.

    2. Multiple publishers after the publisher called `stopOffer()` or is removed
    The last n samples will never be received since they vanished. An arbitrary number of samples or nothing is received.

    For more information about the options see the corresponding example [`iceoptions`](../../../iceoryx_examples/iceoptions/README.md).

!!! info
    If the `PublisherOptions::historyCapacity` is larger than `SubscriberOptions::queueCapacity` and blocking behaviour
    is active, late-joining subscribers will not receive the latest and greatest sample, effectively loosing some.

## Publisher and subscriber matching criteria

If `requiresPublisherHistorySupport` is set, additionally to the matching criteria of server and client, there is a third one for publishers and subscribers:

1. Same `capro::ServiceDescription`
2. Matching `ConsumerTooSlowPolicy` and `QueueFullPolicy` in `PublisherOptions`/`SubscriberOptions`
3. `PublisherOptions::historyCapacity` > 0
