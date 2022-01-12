# QoS policies

Unlike other middlewares like for example [DDS used in ROS 2](https://docs.ros.org/en/galactic/Concepts/About-Quality-of-Service-Settings.html)
iceoryx keeps things simple and offers only basic Quality of Service (QoS) settings.The QoS
settings are called options and can be used to optimize and tailor the communication.

## `PublisherOptions` and `SubscriberOptions`

The three most important settings are similar to the respective DDS setting:

| DDS                       | iceoryx                             | Explanation                                                                                                                    |
|---------------------------|-------------------------------------|--------------------------------------------------------------------------------------------------------------------------------|
| History depth of writer   | `PublisherOptions::historyCapacity` | This enables late-joining subscribers to request the n last samples                                                            |
| History depth of reader   | `SubscriberOptions::queueCapacity`  | Size of the subscriber queue where samples are stored before they are passed to the user                                       |
| -                         | `SubscriberOptions::historyRequest` | The number of samples a late-joining subscriber will request from a publisher, must be equal or smaller than `historyCapacity` |

The history depth of DDS depend on the durability setting. Late-joining readers in DDS
only get values with `Non-Volatile` durability, e.g. `Transient Local`.

For more information about the options see the corresponding example [`iceoptions`](../getting-started/examples/iceoptions.md).

## Publisher subscriber matching criteria

Two criteria have to be fulfilled in order for a publisher and a subscriber to be connected.

 1. Same `capro::ServiceDescription`
 2. Matching `SubscriberTooSlowPolicy` and `QueueFullPolicy` in `PublisherOptions` and `SubscriberOptions`

### Compatible policies

| `SubscriberTooSlowPolicy` | `QueueFullPolicy`     | Result   |
|---------------------------|-----------------------|----------|
| `WAIT_FOR_SUBSCRIBER`     | `BLOCK_PUBLISHER`     | Match    |
| `WAIT_FOR_SUBSCRIBER`     | `DISCARD_OLDEST_DATA` | Match    |
| `DISCARD_OLDEST_DATA`     | `DISCARD_OLDEST_DATA` | Match    |
| `DISCARD_OLDEST_DATA`     | `BLOCK_PUBLISHER`     | No match |
