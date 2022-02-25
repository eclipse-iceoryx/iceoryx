# request response in C

You can find a more detailed description of the C API in the
[iceoryx_binding_c README.md](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_binding_c/README.md).

## Introduction

The behavior and structure is very close to the
[request response C++ example](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/request_response)
so that we explain here only the C API differences and not the underlying mechanisms.

The rough idea is that the client sends to fibonacci numbers to the server which 
then sends the sum of those numbers back.

## Expected Output

## Code walkthrough

### Client Basic

Like with most iceoryx C application we start with:

 * Registering a signal handler
 * Initialize the runtime

<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][register signal handler and init runtime]-->
```c
signal(SIGINT, sigHandler);
signal(SIGTERM, sigHandler);

iox_runtime_init(APP_NAME);
```

We continue with initializing our `client` to send requests to the server. First
of all, the client needs some memory in which the client can be stored called `clientStorage`.
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
in the servers response.

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
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][main loop][loan request]-->
```c
struct AddRequest* request = NULL;
enum iox_AllocationResult loanResult =
    iox_client_loan_request(client, (void**)&request, sizeof(struct AddRequest));
```

To set the sequence id we have to acquire the request header first from the payload. Additionally,
we set `expectedResponseSequenceId` so that we can verify the response later and increment the 
`requestSequenceId` for the next run.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][main loop][set sequence id]-->
```c
iox_request_header_t requestHeader = iox_request_header_from_payload(request);
iox_request_header_set_sequence_id(requestHeader, requestSequenceId);
expectedResponseSequenceId = requestSequenceId;
requestSequenceId += 1;
```

Before we can send out the `request` we have to set the two fibonacci numbers.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][main loop][set and send request]-->
```c
request->augend = fibonacciLast;
request->addend = fibonacciCurrent;
printf("%s Send Request: %lu + %lu\n",
       APP_NAME,
       (unsigned long)fibonacciLast,
       (unsigned long)fibonacciCurrent);
iox_client_send(client, request);
```

Now we give the server a little time to process the `request`.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][main loop][wait for response]-->
```c
const uint32_t DELAY_TIME_IN_MS = 150U;
sleep_for(DELAY_TIME_IN_MS);
```

We process the `response` by acquiring it first with `iox_client_take_response`.
If this is successful we verify the sequence number, adjust our fibonacci numbers
and print our response to the console.
When the sequence number does not fulfill our expectation we print an error message.
<!--[geoffrey][iceoryx_examples/request_response_in_c/client_c_basic.c][main loop][process response]-->
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
successfully we print a info message to the console and loan a `response`.
We require the `request` for the loan so that the `response` can be delivered
to the corresponding client.
When the `iox_server_loan_response` was successful we calculate the sum of the
two received fibonacci numbers and send it.
<!--[geoffrey][iceoryx_examples/request_response_in_c/server_c_basic.c][main loop][process request]-->
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
        iox_server_send(server, response);
    }
    else
    {
        printf("%s Could not allocate Response! Return value = %d\n", APP_NAME, loanResult);
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
