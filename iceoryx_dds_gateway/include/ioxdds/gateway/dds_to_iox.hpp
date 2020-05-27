#pragma once

#include <iceoryx_posh/popo/publisher.hpp>

#include "ioxdds/dds/dds_types.hpp"
#include "ioxdds/gateway/base_gateway.hpp"
#include "ioxdds/gateway/input_channel.hpp"

namespace iox {
namespace dds {

template <typename publisher_t = iox::popo::Publisher,
          typename data_reader_t = iox::dds::data_reader_t>
class DDS2IceoryxGateway : public iox::dds::DDSGateway<iox::dds::InputChannel<publisher_t, data_reader_t>>
{

    using InputChannelFactory = std::function<InputChannel<publisher_t, data_reader_t>(const iox::capro::ServiceDescription)>;
    using InputChannelVector = iox::cxx::vector<InputChannel<publisher_t, data_reader_t>, MAX_CHANNEL_NUMBER>;
    using ConcurrentInputChannelVector = iox::concurrent::smart_lock<InputChannelVector>;

public:
    DDS2IceoryxGateway();
    ~DDS2IceoryxGateway();

    DDS2IceoryxGateway(const DDS2IceoryxGateway&) = delete;
    DDS2IceoryxGateway& operator=(const DDS2IceoryxGateway&) = delete;
    DDS2IceoryxGateway(DDS2IceoryxGateway&&) = delete;
    DDS2IceoryxGateway& operator=(DDS2IceoryxGateway&&) = delete;

    void runMultithreaded() noexcept;
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward() noexcept;
    void shutdown() noexcept;

private:
    std::atomic_bool m_isRunning{false};
    std::atomic_bool m_runForwardingLoop{false};
    std::atomic_bool m_runDiscoveryLoop{false};

    std::thread m_discoveryThread;
    std::thread m_forwardingThread;

    InputChannelFactory m_channelFactory;
    ConcurrentInputChannelVector m_channels;

    void forwardingLoop() noexcept;
    void discoveryLoop() noexcept;

    void loadConfiguration() noexcept;

    InputChannel<publisher_t, data_reader_t> setupChannel(const iox::capro::ServiceDescription& service) noexcept;
    void discardChannel(const iox::capro::ServiceDescription& service) noexcept;

};

} // namespace dds
} // namespace iox

#include "ioxdds/internal/gateway/dds_to_iox.inl"
