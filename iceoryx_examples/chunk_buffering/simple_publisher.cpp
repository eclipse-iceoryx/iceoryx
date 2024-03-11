#include "iox/signal_watcher.hpp"

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

int main()
{
    constexpr char APP_NAME[] = "simple_publisher";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    iox::popo::Publisher<int> publisher({"", "", "data"});

    int ct{0};
    //! [wait for term]
    while (!iox::hasTerminationRequested())
    {
        auto loanResult = publisher.loan();
        if (loanResult.has_value())
        {
            auto &sample = loanResult.value();
            *sample = ++ct;
            sample.publish();
        }
        else
        {
            auto error = loanResult.error();
            std::cerr << "Unable to loan sample, error code: " << error << std::endl;
        }
    }

    return 0;
}
