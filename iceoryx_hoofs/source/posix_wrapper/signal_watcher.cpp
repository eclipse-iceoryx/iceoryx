#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"


namespace iox
{
namespace posix
{
void internalSignalHandler(int) noexcept
{
    auto& instance = SignalWatcher::getInstance();
    instance.m_hasSignalOccurred.store(true);

    for (uint64_t i = 0, currentNumberOfWaiter = instance.m_numberOfWaiters.load(); i < currentNumberOfWaiter; ++i)
    {
        instance.m_semaphore.post().or_else([](auto) {
            std::cerr << "Unable to increment semaphore in signal handler" << std::endl;
            constexpr bool UNABLE_TO_INCREMENT_SEMAPHORE_IN_SIGNAL_HANDLER = false;
            cxx::Ensures(UNABLE_TO_INCREMENT_SEMAPHORE_IN_SIGNAL_HANDLER);
        });
    }
}

SignalWatcher::SignalWatcher() noexcept
    : m_semaphore{std::move(Semaphore::create(CreateUnnamedSingleProcessSemaphore, 0U)
                                .or_else([](auto) {
                                    std::cerr << "Unable to create semaphore for signal watcher" << std::endl;
                                    constexpr bool UNABLE_TO_CREATE_SEMAPHORE_FOR_SIGNAL_WATCHER = false;
                                    cxx::Ensures(UNABLE_TO_CREATE_SEMAPHORE_FOR_SIGNAL_WATCHER);
                                })
                                .value())}
    , m_sigTermGuard(registerSignalHandler(Signal::TERM, internalSignalHandler))
    , m_sigIntGuard(registerSignalHandler(Signal::INT, internalSignalHandler))
{
}

SignalWatcher& SignalWatcher::getInstance() noexcept
{
    static SignalWatcher instance;
    return instance;
}

void SignalWatcher::waitForSignal() const noexcept
{
    ++m_numberOfWaiters;
    if (m_hasSignalOccurred.load())
    {
        return;
    }

    m_semaphore.wait().or_else([](auto) {
        std::cerr << "Unable to wait on semaphore in signal watcher" << std::endl;
        constexpr bool UNABLE_TO_WAIT_ON_SEMAPHORE_IN_SIGNAL_WATCHER = false;
        cxx::Ensures(UNABLE_TO_WAIT_ON_SEMAPHORE_IN_SIGNAL_WATCHER);
    });
}

bool SignalWatcher::wasSignalTriggered() const noexcept
{
    return m_hasSignalOccurred.load();
}

void waitForTerminationRequest() noexcept
{
    SignalWatcher::getInstance().waitForSignal();
}

bool hasTerminationRequested() noexcept
{
    return SignalWatcher::getInstance().wasSignalTriggered();
}

} // namespace posix
} // namespace iox
