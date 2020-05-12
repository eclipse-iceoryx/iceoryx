#pragma once

#include <cstdint>

#include <iceoryx_posh/iceoryx_posh_types.hpp>
#include <iceoryx_posh/popo/subscriber.hpp>
#include <iceoryx_utils/internal/objectpool/objectpool.hpp>

#include "ioxdds/dds/data_writer.hpp"
#include "ioxdds/dds/dds_types.hpp"

namespace iox
{
namespace gateway
{
namespace dds
{

static constexpr uint32_t MAX_CHANNEL_NUMBER = MAX_PORT_NUMBER;


///
/// @brief Groups and manages resources that make up a posh->dds channel
///
template <typename subscriber_t = iox::popo::Subscriber,
          typename data_writer_t = iox::dds::data_writer_t>
class Channel {
public:

    using SubscriberPtr = std::shared_ptr<subscriber_t>;
    using SubscriberPool = iox::cxx::ObjectPool<subscriber_t, MAX_CHANNEL_NUMBER>;
    using DataWriterPtr = std::shared_ptr<data_writer_t>;
    using DataWriterPool = iox::cxx::ObjectPool<data_writer_t, MAX_CHANNEL_NUMBER>;

    ///
    /// @brief Channel Constructs an object with unmanaged resources.
    /// @param service The service that the channel is connecting.
    /// @param subscriber An externally managed subscriber endpoint.
    /// @param dataWriter An externally managed data writer endpoint.
    ///
    Channel(const iox::capro::ServiceDescription& service, SubscriberPtr subscriber, DataWriterPtr dataWriter);

    ///
    /// @brief create Creates a channel with internally managed resources.
    /// @param service The service that the channel is connecting.
    /// @return Channel A channel with internally managed endpoints.
    ///
    static Channel create(const iox::capro::ServiceDescription& service);

    iox::capro::ServiceDescription getService();
    SubscriberPtr getSubscriber();
    DataWriterPtr getDataWriter();

private:
    // Store in data segment - too large to keep in stack.
    static SubscriberPool s_subscriberPool;
    static DataWriterPool s_dataWriterPool;

    iox::capro::ServiceDescription service;
    SubscriberPtr subscriber;
    DataWriterPtr dataWriter;
};

} // dds
} // gateway
} // iox

#include "ioxdds/internal/gateway/channel.inl"

