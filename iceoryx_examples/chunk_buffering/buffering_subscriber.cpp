#include "iox/signal_watcher.hpp"

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include <deque>

#include <chrono>

struct DrainBufferStats
{
    std::chrono::nanoseconds total{0};
    std::chrono::nanoseconds min{0};
    std::chrono::nanoseconds max{0};
    std::chrono::nanoseconds avg{0};
};

int main()
{
    constexpr char APP_NAME[] = "buffering_subscriber";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // The subscriber must be able to hold 10 messages in its queue and one message in the local call stack.
    constexpr uint64_t queue_capacity = 10U;
    constexpr uint64_t max_buffer_size = iox::MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY - queue_capacity - 1U;

    iox::popo::SubscriberOptions subscriber_options;
    subscriber_options.queueCapacity = queue_capacity;
    iox::popo::Subscriber<int> subscriber({"", "", "data"}, subscriber_options);

    std::cout << "The current buffer size is: " << max_buffer_size << std::endl;
    std::deque<iox::popo::Sample<const int, const iox::mepoo::NoUserHeader>> buffered_samples;

    uint64_t run_count{0};
    while (!iox::hasTerminationRequested())
    {
        auto take_result = subscriber.take();
        if (take_result.has_value())
        {
            if (buffered_samples.size() == max_buffer_size)
            {
                DrainBufferStats stats{};
                // Profile how long it takes to drain the queue.
                while (!buffered_samples.empty())
                {
                    // Change this value to observe the difference between releasing new and old chunks.
                    constexpr bool release_oldest{true};
                    
                    auto start = std::chrono::steady_clock::now();
                    if constexpr (release_oldest)
                    {
                        buffered_samples.pop_front();
                    }
                    else
                    {
                        buffered_samples.pop_back();
                    }
                    auto stop = std::chrono::steady_clock::now();
                    auto elapsed_time{std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start)};
                    
                    stats.total += elapsed_time;
                    stats.max = std::max(stats.max, elapsed_time);
                    stats.min = std::min(stats.min, elapsed_time);
                }
                stats.avg = stats.total / max_buffer_size;

                std::cout << "Stats from run " << ++run_count << ": " << std::endl;
                std::cout << std::endl;
                std::cout << "Total time elapsed (ns): " << std::chrono::duration_cast<std::chrono::nanoseconds>(stats.total).count() << std::endl;
                std::cout << "Max release time (ns): " << stats.max.count() << std::endl;
                std::cout << "Min release time (ns): " << stats.min.count() << std::endl;
                std::cout << "Avg release time (ns): " << stats.avg.count() << std::endl;
                std::cout << std::endl << std::endl;
            }

            buffered_samples.push_back(std::move(take_result).value());
        }
    }
}

