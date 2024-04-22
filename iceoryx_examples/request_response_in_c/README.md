# Request response in C

You can find a more detailed description of the C API in the
[iceoryx_binding_c README.md](../../iceoryx_binding_c/README.md).

## Introduction

The behavior and structure is very close to the
[request response C++ example](../request_response)
so that we explain here only the C API differences and not the underlying mechanisms.

The rough idea is that the client sends two fibonacci numbers to the server which
then sends the sum of those numbers back.

## Expected Output

[![asciicast](https://asciinema.org/a/476845.svg)](https://asciinema.org/a/476845)

## Code walkthrough

### Client Basic

Like with most iceoryx C applications we start with:

 * Registering a signal handler
 * Initialize the runtime

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][register signal handler and init runtime]-->
```c
signal(SIGINT, sigHandler);
signal(SIGTERM, sigHandler);

iox_runtime_init(APP_NAME);
```

The signal handler clears a flag to initiate a graceful shutdown.

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][signal handler]-->
```c
volatile bool keepRunning = true;

void sigHandler(int signalValue)
{
    (void)signalValue;
    keepRunning = false;
}
```

We continue with initializing our `client` to send requests to the server. First
of all, we need some memory in which the client can be stored called `clientStorage`.
`iox_client_init` will create an object in this memory location, sets the service
description and uses the default client options which is indicated by the `NULL` argument
at the end.

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][create client]-->
```c
iox_client_storage_t clientStorage;
iox_client_t client = iox_client_init(&clientStorage, "Example", "Request-Response", "Add", NULL);
```

Like in the C++ version of our example we implement a client/server based fibonacci
algorithm and store the first two fibonacci numbers in `fibonacciLast` and `fibonacciCurrent`.
`requestSequenceId` and `expectedResponseSequenceId` are helping us to keep track of
the sequence number which we increase with every sent request and then verify it again
in the server's response.

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][define variables]-->
```c
uint64_t fibonacciLast = 0U;
uint64_t fibonacciCurrent = 1U;
int64_t requestSequenceId = 0;
int64_t expectedResponseSequenceId = requestSequenceId;
```

We enter our main loop and continue sending requests as long as `keepRunning` is `true`.
When the users presses Control+C the term signal is emitted and `keepRunning` is set to
`false` via the signal handler callback.

We start by loaning memory to send out our `request`.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][loan request]-->
```c
struct AddRequest* request = NULL;
enum iox_AllocationResult loanResult =
    iox_client_loan_request(client, (void**)&request, sizeof(struct AddRequest));
```

To set the sequence id we have to acquire the request header first from the payload. Additionally,
we set `expectedResponseSequenceId` so that we can verify the response later and increment the
`requestSequenceId` for the next run.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][set sequence id]-->
```c
iox_request_header_t requestHeader = iox_request_header_from_payload(request);
iox_request_header_set_sequence_id(requestHeader, requestSequenceId);
expectedResponseSequenceId = requestSequenceId;
requestSequenceId += 1;
```

Before we can send out the `request` we have to set the two fibonacci numbers.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][set and send request]-->
```c
request->augend = fibonacciLast;
request->addend = fibonacciCurrent;
printf("%s Send Request: %lu + %lu\n",
       APP_NAME,
       (unsigned long)fibonacciLast,
       (unsigned long)fibonacciCurrent);
enum iox_ClientSendResult sendResult = iox_client_send(client, request);
if (sendResult != ClientSendResult_SUCCESS)
{
    printf("Error sending Request! Error code: %d\n", sendResult);
}
```

Now we give the server a little time to process the `request`.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][wait for response]-->
```c
const uint32_t DELAY_TIME_IN_MS = 150U;
sleep_for(DELAY_TIME_IN_MS);
```

We process the `response` by acquiring it first with `iox_client_take_response`.
If this is successful we verify the sequence number, adjust our fibonacci numbers
and print our response to the console.
When the sequence number does not fulfill our expectation we print an error message.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][process response]-->
```c
const struct AddResponse* response = NULL;
while (iox_client_take_response(client, (const void**)&response) == ChunkReceiveResult_SUCCESS)
{
    iox_const_response_header_t responseHeader = iox_response_header_from_payload_const(response);
    int64_t receivedSequenceId = iox_response_header_get_sequence_id_const(responseHeader);
    if (receivedSequenceId == expectedResponseSequenceId)
    {
        fibonacciLast = fibonacciCurrent;
        fibonacciCurrent = response->sum;
        printf("%s Got Response: %lu\n", APP_NAME, (unsigned long)fibonacciCurrent);
    }
    else
    {
        printf("Got Response with outdated sequence ID! Expected = %lu; Actual = %lu! -> skip\n",
               (unsigned long)expectedResponseSequenceId,
               (unsigned long)receivedSequenceId);
    }

    iox_client_release_response(client, response);
}
```

Please do not forget to call `iox_client_release_response` to release the `response`
again. If you forget this you won't be able to receive `response`s anymore after a
certain time since you hold to many responses in parallel.

As final step we cleanup the used resources and deinitialize the client.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][cleanup]-->
```c
iox_client_deinit(client);
```

### Client WaitSet

The server and client or both attachable to either a listener or a waitset. In
this example we demonstrate how one can implement the client basic example with
a waitset.
For deeper insights into the WaitSet take a look at the
[WaitSet C++ example](../waitset)
or when you would like to know more about the listener, see the
[Callbacks C++ example](../callbacks).

The startup phase is identical to the client basic version, we register the signal
handlers, initialize the runtime, create a client and initialize our variables.

This time the signal handler needs to wake up the waitset additionally.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_waitset.c][signal handler]-->
```c
volatile bool keepRunning = true;

volatile iox_ws_t waitsetSigHandlerAccess = NULL;

void sigHandler(int signalValue)
{
    (void)signalValue;
    keepRunning = false;
    if (waitsetSigHandlerAccess)
    {
        iox_ws_mark_for_destruction(waitsetSigHandlerAccess);
    }
}
```

Afterwards we create our waitset and attach the client state `ClientState_HAS_RESPONSE`
to it.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_waitset.c][create waitset and attach client]-->
```c
iox_ws_storage_t waitsetStorage;
iox_ws_t waitset = iox_ws_init(&waitsetStorage);
waitsetSigHandlerAccess = waitset;

if (iox_ws_attach_client_state(waitset, client, ClientState_HAS_RESPONSE, 0U, NULL) != WaitSetResult_SUCCESS)
{
    printf("failed to attach client\n");
    _exit(-1);
}
```

Again we perform the same task like in the client basic example. We enter our
main loop, loan a request and set it up.
But after we sent the request to our server we do not sleep for some time, we wait
on the waitset until the request was received.
We use `iox_ws_timed_wait` to wait for at most 2 seconds.

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_waitset.c][wait for response]-->
```c
iox_notification_info_t notificationArray[NUMBER_OF_NOTIFICATIONS];
uint64_t missedNotifications = 0U;
struct timespec timeout;
timeout.tv_sec = 2;
timeout.tv_nsec = 0;

uint64_t numberOfNotifications =
    iox_ws_timed_wait(waitset, timeout, notificationArray, NUMBER_OF_NOTIFICATIONS, &missedNotifications);
```

When this blocking call
returns we iterate over the `notificationArray` and when one triggered originated
from our `client` we acquire all responses in a while loop and print them to
the console.

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_waitset.c][process responses]-->
```c
for (uint64_t i = 0; i < numberOfNotifications; ++i)
{
    if (iox_notification_info_does_originate_from_client(notificationArray[i], client))
    {
        const struct AddResponse* response = NULL;
        while (iox_client_take_response(client, (const void**)&response) == ChunkReceiveResult_SUCCESS)
        {
            iox_const_response_header_t responseHeader = iox_response_header_from_payload_const(response);
            int64_t receivedSequenceId = iox_response_header_get_sequence_id_const(responseHeader);
            if (receivedSequenceId == expectedResponseSequenceId)
            {
                fibonacciLast = fibonacciCurrent;
                fibonacciCurrent = response->sum;
                printf("%s Got Response: %lu\n", APP_NAME, (unsigned long)fibonacciCurrent);
            }
            else
            {
                printf("Got Response with outdated sequence ID! Expected = %lu; Actual = %lu! -> skip\n",
                       (unsigned long)expectedResponseSequenceId,
                       (unsigned long)receivedSequenceId);
            }
            iox_client_release_response(client, response);
        }
    }
}
```

The cleanup is done when we exit our mainloop. We detach the client state from
the waitset first and then deinitialize the waitset and the client.

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_waitset.c][cleanup]-->
```c
iox_ws_detach_client_state(waitset, client, ClientState_HAS_RESPONSE);
waitsetSigHandlerAccess = NULL; // invalidate for signal handler
iox_ws_deinit(waitset);
iox_client_deinit(client);
```

### Server Basic

We again start with registering the signal handler and the runtime.

<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_basic.c][register signal handler and init runtime]-->
```c
signal(SIGINT, sigHandler);
signal(SIGTERM, sigHandler);

iox_runtime_init(APP_NAME);
```

As next step we initialize the `server`. Like the client the `server` requires
some memory where it can be stored, the `serverStorage`. The `server` is also
initialized with the default options which is indicated via the last `NULL`
argument.
<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_basic.c][init server]-->
```c
iox_server_storage_t serverStorage;
iox_server_t server = iox_server_init(&serverStorage, "Example", "Request-Response", "Add", NULL);
```

We enter the main loop and start it by taking a `request`. If it was taken
successfully, we print an info message to the console and loan a `response`.
We require the `request` for the loan so that the `response` can be delivered
to the corresponding client.
When the `iox_server_loan_response` was successful we calculate the sum of the
two received fibonacci numbers and send it.
<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_basic.c][process request]-->
```c
const struct AddRequest* request = NULL;
if (iox_server_take_request(server, (const void**)&request) == ServerRequestResult_SUCCESS)
{
    printf("%s Got Request: %lu + %lu\n",
           APP_NAME,
           (unsigned long)request->augend,
           (unsigned long)request->addend);

    struct AddResponse* response = NULL;
    enum iox_AllocationResult loanResult =
        iox_server_loan_response(server, request, (void**)&response, sizeof(struct AddResponse));
    if (loanResult == AllocationResult_SUCCESS)
    {
        response->sum = request->augend + request->addend;
        printf("%s Send Response: %lu\n", APP_NAME, (unsigned long)response->sum);
        enum iox_ServerSendResult sendResult = iox_server_send(server, response);
        if (sendResult != ServerSendResult_SUCCESS)
        {
            printf("Error sending Response! Error code: %d\n", sendResult);
        }
    }
    else
    {
        printf("%s Could not allocate Response! Error code: %d\n", APP_NAME, loanResult);
    }

    iox_server_release_request(server, request);
}
```

Again, it is important that one releases the `request` with `iox_server_release_request`
otherwise `iox_server_take_request` will fail since one holds to many requests in parallel.

The final step is again the resource cleanup where we deinitialize the server.

<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_basic.c][cleanup]-->
```c
iox_server_deinit(server);
```

### Server Listener

The server and client or both attachable to either a listener or a waitset. In
this example we demonstrate how one can implement the server basic example with
a listener.
For deeper insights into the WaitSet take a look at the
[WaitSet C++ example](../waitset)
or when you would like to know more about the listener, see the
[Callbacks C++ example](../callbacks).

The listener example starts like the basic example by registering the signal handler,
initializing the runtime and creating a server.

In the next step we create a listener and attach the server event `ServerEvent_REQUEST_RECEIVED`
with `iox_listener_attach_server_event`. One parameter of that call is `onRequestReceived` a
pointer to a function which handles the logic whenever we receive a request.
<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_listener.c][create and attach to listener]-->
```c
iox_server_storage_t serverStorage;
iox_server_t server = iox_server_init(&serverStorage, "Example", "Request-Response", "Add", NULL);

iox_listener_storage_t listenerStorage;
iox_listener_t listener = iox_listener_init(&listenerStorage);

if (iox_listener_attach_server_event(listener, server, ServerEvent_REQUEST_RECEIVED, onRequestReceived)
    != ListenerResult_SUCCESS)
{
    printf("unable to attach server\n");
    _exit(-1);
}
```

In the next step we run into our main loop which waits until the user terminates the process.
<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_listener.c][mainloop]-->
```c
while (keepRunning)
{
    const uint32_t SLEEP_TIME_IN_MS = 500U;
    sleep_for(SLEEP_TIME_IN_MS);
}
```

The actual logic behind processing a request is handled in the function `onRequestReceived`
which is called in a thread inside the listener whenever our server receives a request.
The code again looks identical to the server basic example. We take the request with
`iox_server_take_request`, print a message to the console and then loan a response
with `iox_server_loan_response` which is populated and send to the client with
`iox_server_send`.
<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_listener.c][process request]-->
```c
void onRequestReceived(iox_server_t server)
{
    const struct AddRequest* request = NULL;
    while (iox_server_take_request(server, (const void**)&request) == ServerRequestResult_SUCCESS)
    {
        printf("%s Got Request: %lu + %lu\n", APP_NAME, (unsigned long)request->augend, (unsigned long)request->addend);

        struct AddResponse* response = NULL;
        enum iox_AllocationResult loanResult =
            iox_server_loan_response(server, request, (void**)&response, sizeof(struct AddResponse));
        if (loanResult == AllocationResult_SUCCESS)
        {
            response->sum = request->augend + request->addend;
            printf("%s Send Response: %lu\n", APP_NAME, (unsigned long)response->sum);
            enum iox_ServerSendResult sendResult = iox_server_send(server, response);
            if (sendResult != ServerSendResult_SUCCESS)
            {
                printf("Error sending Response! Error code: %d\n", sendResult);
            }
        }
        else
        {
            printf("Could not allocate Response! Error code: %d\n", loanResult);
        }
        iox_server_release_request(server, request);
    }
}
```

The resource cleanup is done after our mainloop has ended. We detach the server event from
the listener first and then deinitialize the listener and server.
<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_listener.c][cleanup]-->
```c
iox_listener_detach_server_event(listener, server, ServerEvent_REQUEST_RECEIVED);
iox_listener_deinit(listener);
iox_server_deinit(server);
```

<center>
[Check out request response in c on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/request_response_in_c){ .md-button } <!--NOLINT github url required for website-->
</center>
