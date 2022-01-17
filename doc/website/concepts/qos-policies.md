# QoS policies

Unlike other middlewares, e.g. [DDS used in ROS 2](https://docs.ros.org/en/galactic/Concepts/About-Quality-of-Service-Settings.html),
iceoryx keeps things simple and offers basic Quality of Service (QoS) settings. The QoS
settings are called options and can be used to optimize and tailor the communication.

## `PublisherOptions` and `SubscriberOptions`

The three most important settings are:

| Option                              | Explanation                                                                                                                    |
|-------------------------------------|--------------------------------------------------------------------------------------------------------------------------------|
| `PublisherOptions::historyCapacity` | This enables late-joining subscribers to request the n last samples                                                            |
| `SubscriberOptions::queueCapacity`  | Size of the subscriber queue where samples are stored before they are passed to the user                                       |
| `SubscriberOptions::historyRequest` | The number of samples a late-joining subscriber will request from a publisher, must be equal or smaller than `historyCapacity` |

For more information about the options see the corresponding example [`iceoptions`](../getting-started/examples/iceoptions.md).

## Publisher subscriber matching criteria

Two criteria have to be fulfilled in order for a publisher and a subscriber to be connected.

 1. Same `capro::ServiceDescription`
 2. Matching `SubscriberTooSlowPolicy` and `QueueFullPolicy` in `PublisherOptions` and `SubscriberOptions`

### Compatible policies

| `SubscriberTooSlowPolicy` | `QueueFullPolicy`     | Behavior                                  | Connection            |
|---------------------------|-----------------------|-------------------------------------------|-----------------------|
| `WAIT_FOR_SUBSCRIBER`     | `BLOCK_PUBLISHER`     | Publisher blocks and waits for subscriber | :white_check_mark:    |
| `WAIT_FOR_SUBSCRIBER`     | `DISCARD_OLDEST_DATA` | Non-blocking publisher                    | :white_check_mark:    |
| `DISCARD_OLDEST_DATA`     | `DISCARD_OLDEST_DATA` | Non-blocking publisher                    | :white_check_mark:    |
| `DISCARD_OLDEST_DATA`     | `BLOCK_PUBLISHER`     | Not compatible, no connection established | :x:                   |
