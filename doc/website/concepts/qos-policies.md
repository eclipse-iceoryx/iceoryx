# QoS policies

Unlike other middlewares like for example [DDS used in ROS 2](https://docs.ros.org/en/galactic/Concepts/About-Quality-of-Service-Settings.html) iceoryx tries to keep things simple and offers only basic Quality of Service (QoS) settings. The QoS settings are called options and can be used to optimize and tailor the communication.

## `PublisherOptions` and `SubscriberOptions`

The three most important settings, which are also available in DDS, are:

* `PublisherOptions::historyCapacity`: This will enable late-joining subscribers to read the n last samples
* `SubscriberOptions::queueCapacity`: Size of the subscriber queue where samples are stored before they are passed to the user
* `SubscriberOptions::historyRequest`: The number of samples a late-joining subscriber will request from a publisher

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

