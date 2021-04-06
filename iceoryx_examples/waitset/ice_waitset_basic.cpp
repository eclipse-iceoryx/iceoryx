#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
#include "topic_data.hpp"

#include <iostream>
#include <atomic>

std::atomic_bool shutdown{false};
iox::popo::UserTrigger shutdownTrigger;

static void sigHandler(int sig [[gnu::unused]])
{
    shutdown = true;
    shutdownTrigger.trigger();
}

int main()
{
    // register signal handler to handle termination of the loop
    auto signalGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime("my_app");

    // create subscriber
    iox::popo::Subscriber<CounterTopic> subscriber({"Radar", "FrontLeft", "Counter"});
    
    // create waitset 
    // it needs to be able to manage 2 triggers (subscriber and shutdown)
    iox::popo::WaitSet<2> waitset;

    // attach shutdown trigger to waitset (needed to stop the processing loop)
    waitset.attachEvent(shutdownTrigger).or_else([](auto) {
        std::cerr << "failed to attach shutdown trigger" << std::endl;
        std::terminate();
    });

    // attach subscriber to waitset
    waitset.attachState(subscriber, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
        std::cerr << "failed to attach subscriber" << std::endl;
        std::terminate();
    });

    while (true)
    {
        // We block and wait for samples to arrive.
        waitset.wait();

        // We woke up and hence there must be at least one sample or a shutdown request.
        // We know which subscriber received samples since we only attached one.
        // Otherwise we would need to check the reason for the wake-up by inspecting the return value.
        // This requires iterating over the result vaector but an be avoided in this simple case.

        if(shutdown) {
            std::cout << "shutting down" << std::endl;
            break; // the shutdown trigger must have set this and we leave the loop
        }

        // No shutdown requested, hence we know which the only attached subscriber should have data. 

        // Consume a sample
        subscriber.take()
             .and_then([](auto& sample) { std::cout << " got value: " << sample->counter << std::endl; })
             .or_else([](auto& reason) { (void) reason; /* we could check and handle the reason why there is no data */ 
                         std::cout << "got no data" << std::endl;});

        // We could consume all samples but do not need to.
        // If there is more than one sample we will wake up again since the state of the subscriber is still
        // iox::popo::SubscriberState::HAS_DATA in this case.
    }

    return (EXIT_SUCCESS);
}