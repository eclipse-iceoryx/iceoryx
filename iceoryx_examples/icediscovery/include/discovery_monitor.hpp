#pragma once

#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"

namespace discovery
{
using ServiceDiscovery = iox::runtime::ServiceDiscovery;

ServiceDiscovery& serviceDiscovery()
{
    static ServiceDiscovery instance;
    return instance;
}

class DiscoveryMonitor
{
  public:
    DiscoveryMonitor()
        : m_discovery(&serviceDiscovery())
    {
    }

    template <typename Callback>
    bool registerCallback(Callback callback)
    {
        if (m_callback)
        {
            return false;
        }

        m_callback = callback;

        auto errorHandler = [](auto) {
            std::cerr << "failed to attach to listener" << std::endl;
            std::exit(EXIT_FAILURE);
        };

        auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
        m_listener.attachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
            .or_else(errorHandler);

        return true;
    }

    // no more information needed since there is at most one callback
    void deregisterCallback()
    {
        if (m_callback)
        {
            m_listener.detachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED);
        }
        m_callback = nullptr;
    }

  private:
    using callback_t = iox::cxx::function<void(ServiceDiscovery&)>;

    ServiceDiscovery* m_discovery{nullptr};

    // no reason to use a pointer here, the same listener can attach to the registry
    // only once (per event but there is only one)
    iox::popo::Listener m_listener;

    callback_t m_callback;

    static void invokeCallback(ServiceDiscovery* discovery, DiscoveryMonitor* self)
    {
        self->m_callback(*discovery);
    }
};

} // namespace discovery
