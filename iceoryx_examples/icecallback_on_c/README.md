# [NOT YET IMPLEMENTED] icecallback_on_c - Using the WaitSet to implement callbacks

**Hint** This example shows a work in progress and is not yet working.

### The WaitSet

Let's say you would like to send increasing numbers from a publisher to a 
subscriber which should print the received numbers to the terminal. The classical
approach would be to let the subscriber run in a `while-true` loop and check
with every iteration if there is new data.

This approach has the drawback that you either burn the CPU but you will most likely
receive the your data instantly or you wait in every iteration. This could cause
the subscriber to receive the data with a latency. The `WaitSet` solves this problem. 

The `WaitSet` is a set of conditions which
provides you with the ability to wait until one or more of those conditions are 
triggered. A condition could be for instance a subscriber which is triggered when
a new sample arrives. Another condition is called `GuardCondition` which can 
be triggered by the user to signal user defined events like process termination.

### Code Walkthrough
In our example the `ice_c_callback_publisher.c` has no new or additional features
and is already explained in detail in the [icedelivery C example](../icedelivery_on_c/).

#### Subscriber

To use a `WaitSet` we perform the following steps:

1. Initialize a `WaitSet` and optionally a `GuardCondition`.
2. Attach the conditions to our `WaitSet`.
3. Wait for a condition to be triggered. 
    - If the `GuardCondition` is triggered we terminate the process.
    - If the subscriber was triggered we receive and print the new data.
4. Before we cleanup the subscriber and the `GuardCondition` we have to detach them
    from the `WaitSet`.
5. Cleanup `WaitSet`

Now let's take a look at the details by skipping the already known statements and
focus on the `WaitSet` specific part:

1. First we define our `GuardCondition`, `WaitSet` as global variables on the 
   top of our file.
   Additionally, we need some place to store them, this is why we declare 
   the `guardConditionStorage` and `waitSetStorage` so that this can be allocated
   directly on the stack.
    ```c
    iox_guard_cond_storage_t guardConditionStorage;
    iox_guard_cond_t guardCondition;

    iox_ws_storage_t waitSetStorage;
    iox_ws_t waitSet;
    ```

   In the `receiving` function we initialize the then the `WaitSet` and the 
   `GuardCondition`.
    ```c
    waitSet = iox_ws_init(&waitSetStorage);
    guardCondition = iox_guard_cond_init(&guardConditionStorage);
    ```

2. After the subscriber is created and the subscribe call is executed we 
   attach the `GuardCondition` and the subscriber to the `WaitSet`.
   ```c
   iox_ws_attach_condition(waitSet, (iox_cond_t)guardCondition);
   iox_ws_attach_condition(waitSet, (iox_cond_t)subscriber);
   ```

3. The `iox_ws_wait` and `iox_ws_timed_wait` calls are writing
   all condition which were triggered into a preallocated array. Furthermore,
   if the array has not sufficient size the number of missed elements is
   written into a number. At first we define the array, the integer in
   which the number of missed elements are stored and a place for the return
   value which signals us how many conditions were actually triggered.
   ```c
   iox_cond_t conditionArray[NUMBER_OF_CONDITIONS];
   uint64_t missedElements = 0U;
   uint64_t numberOfTriggeredConditions = 0U;
   ```

   In a loop we wait until the `WaitSet` is triggered and then handle the
   condition in our `callback` function. If the `callback` function returns
   false it signals us that the `GuardCondition` was triggered and that we
   should clean up our resources.
   ```c
   do
   {
       numberOfTriggeredConditions = 
             iox_ws_wait(waitSet, conditionArray, NUMBER_OF_CONDITIONS, 
                               &missedElements);
   } while (callback(conditionArray, numberOfTriggeredConditions));
   ```

4. Before we can cleanup the subscriber and the `GuardCondition` we have to
   detach them from the `WaitSet`. This can be done either by detaching all
   conditions with `iox_ws_detach_all_conditions` or by calling
   `iox_ws_detach_condition` for every condition.
   ```c
   iox_ws_detach_all_conditions(waitSet);
   ```

5. And finally we can cleanup our resources.
   ```c
   iox_ws_deinit(waitSet);
   iox_guard_cond_deinit(guardCondition);
   ```

#### Callback
In our callback we handle our reaction to an event. We will be provided with
the array which contains all conditions which were triggered. This allows us
to iterate through this array, look if the condition was either our 
`GuardCondition` or our subscriber and then act accordingly.
```c
for (uint64_t i = 0; i < numberOfConditions; ++i)
{
    if (conditions[i] == (iox_cond_t)guardCondition)
    {
        printf("Received exit signal!\n");
        return false;
    }
```
If its our `GuardCondition` we return false to signal the loop that the
process should be terminated.

```c
else if (conditions[i] == (iox_cond_t)subscriber)
{
    if (SubscribeState_SUBSCRIBED == iox_sub_get_subscription_state(subscriber))
    {
        const void* chunk = NULL;
        while (ChunkReceiveResult_SUCCESS == iox_sub_get_chunk(subscriber, &chunk))
        {
            const struct TopicData* sample = (const struct TopicData*)(chunk);
            printf("Receiving: %s\n", sample->message);
            iox_sub_release_chunk(subscriber, chunk);
        }
    }
```
Did the subscriber an event we check if it is still subscribed and if so we
check if the subscriber has received new data. Then we follow the 
`icedelivery` example and print the contents of the sample to the terminal.
