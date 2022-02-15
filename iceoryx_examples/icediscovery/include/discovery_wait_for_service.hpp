#pragma once

#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

namespace discovery
{
using ServiceDiscovery = iox::runtime::ServiceDiscovery;

ServiceDiscovery& serviceDiscovery()
{
    static ServiceDiscovery instance;
    return instance;
}

// user defined discovery functionality
class Discovery
{
  public:
    Discovery()
        : m_discovery(&serviceDiscovery())
    {
        update();
        auto errorHandler = [](auto) {
            std::cerr << "failed to attach to waitset" << std::endl;
            std::terminate();
        };

        m_waitset.attachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED)
            .or_else(errorHandler);
    }

    // returns true if condition was true
    template <typename Condition>
    bool waitUntil(const Condition& discoveryCondition)
    {
        update();
        do
        {
            // 1) we have (almost) current discovery data
            // condition holds?
            bool result = discoveryCondition();
            if (result)
            {
                // 2) condition held and we return (without mutex to protect condition changes
                // there is no way to guarantee it still holds)
                return true;
            }
            else
            {
                if (!m_blocking)
                {
                    return false;
                }
            }
            // 3) condition did not hold but it may hold if we use the latest discovery data
            //    which may have arrived in the meantime

            // 4) this does not wait if there is new discovery data (and hence we try again immediately)
            waitUntilChange();
            // 5) discovery data changed, check condition again (even if unblocked)
        } while (true);

        return false;
    }

    void waitUntilChange()
    {
        m_waitset.wait();
    }

    // unblock any wait, not reversible
    void unblockWait()
    {
        m_blocking = false;
        // could also unblock with a dedicated trigger to break the wait but that requires more code
        // and is not necessary if it is only supposed to happen once
        m_waitset.markForDestruction();
    }


    auto findService(const iox::cxx::optional<iox::capro::IdString_t>& service,
                     const iox::cxx::optional<iox::capro::IdString_t>& instance,
                     const iox::cxx::optional<iox::capro::IdString_t>& event) noexcept
    {
        return m_discovery->findService(service, instance, event);
    }

  private:
    ServiceDiscovery* m_discovery{nullptr};
    iox::popo::WaitSet<1> m_waitset;
    bool m_blocking{true};

    void update()
    {
        // not stritcly required, deending on use case
        m_discovery->update();
    }
};

} // namespace discovery
