# Request/Response Communication

## Summary

Beside the publish-subscribe messaging pattern initially supported by iceoryx,
the request-response pattern is also widely used in the field where iceoryx is headed.
Since we aim for a good integration with ROS 2, this is also necessary to support its
`Service` communication method which is often used to control robots.

While publish-subscribe is a data distribution pattern which continuously delivers
updates on its data, request-response is a remote procedure call and task distribution pattern
which delivers data only on demand. This means there is one or several clients
requesting a server to perform a task.

In iceoryx, the client and server will be built on top of the same building blocks
we already use for publish-subscribe, like the `ChunkSender` and `ChunkReceiver`.

## Terminology

| Name              | Description                                              |
| :---------------- | :------------------------------------------------------- |
| Request-Response  | communication pattern which delivers data on demand      |
| Publish-Subscribe | communication pattern which continuously delivers updates on its data |
| RPC               | a remote procedure call to offload task to a server      |
| Client            | an entity invoking RPCs                                  |
| Server            | an entity processing RPCs                                |

## Design

### Considerations

Client and server shall be able to be used in combination with the `WaitSet` and `Listener`
to provide a means of event based communication instead of polling.

Like with publish-subscribe, the `ServiceDescription` will be used to connect clients with a corresponding server.

In order to support asynchronous requests, a sequence ID should be part of each request which will be copied to the corresponding response.

### Solution

This is an overview of the untyped `Client` and `Server` classes.

![simple class diagram](diagrams/request_response_overview_class.svg)

The `Client` and `Server` are reusing the `ChunkSender` and `ChunkReceiver` building blocks. The `Client` uses a `ChunkSender` to send requests and a `ChunkReceiver` to get the responses while the `Server` uses a `ChunkReceiver` to get the requests and a `ChunkSender` to send the responses.

#### Typed API

![typed API](diagrams/request_response_typed_api.svg)

#### Untyped API

![untyped API](diagrams/request_response_untyped_api.svg)

#### Client Port

![client port](diagrams/request_response_client_port.svg)

The `ClientPortData` is located in the shared memory and contain only the data but no methods to access them.
`ClientPortUser` is the class providing the methods for the user access and `ClientPortRouDi` provides the
interface RouDi needs to connect the client to the server and to cleanup the port resources.

#### Server Port

![server port](diagrams/request_response_server_port.svg)

Similar to the Client Port, the Server Port has `ServerPortData` which is located in the shared memory and contain only the data but no methods to access them.
`ServerPortUser` is the class providing the methods for the user access and `ServerPortRouDi` provides the
interface RouDi needs to connect the client to the server once the server offers its service and to cleanup the port resources.

#### Request/Response Header

![rpc header](diagrams/request_response_header.svg)

Since request and response need to encode different meta-information, we also need different header for the messages.
The common data is aggregated in `RpcBaseHeader` which contains a `RelativePointer` to the `ClientChunkQueueData_t` and a sequence ID.
The `ClientChunkQueueData_t` pointer is used to identify the queue which receives the response.
It must not be used to send the response since the lifetime might already have ended at the time
the response is sent but used as identifier for `ChunkDistributor::deliverToQueue`.
The sequence ID is used to match a response to a specific request if multiple requests are invoke asynchronously.
Depending on the client and server options, a request might be dropped or a server could have a worker pool
which results in sending the responses in a different order than the request were received.
The sequence ID must be set by the user and also checked by the user on response.
The `RequestHeader` has also the option to specify a message as fire and forget, which means it won't get a response to this request.

#### Client/Server Options

![client and server options](diagrams/request_response_options.svg)

The client and server options can be used to control certain aspects of the clients and servers.
Beside setting the capacity of the queues and defining whether a client should be connected and a server offering on creation,
the behaviour for a slow client and server can be defined.
A client can ask a server to block if its response queue is full.
The server will obey if the corresponding `ClientTooSlowPolicy` is set to `WAIT_FOR_CLIENT`.
If the options don't match, the client will not be connected to the server.

### Code example

## Open issues

- integration into a gateway, e.g. the DDS gateway
- check if a Listener can be used for a timed wait for a response on the client side
