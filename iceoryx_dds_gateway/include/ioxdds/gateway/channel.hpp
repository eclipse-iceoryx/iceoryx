#pragma once

#include <memory>

#include <iceoryx_posh/capro/service_description.hpp>
#include <iceoryx_utils/internal/objectpool/objectpool.hpp>
#include "ioxdds/dds/dds_configs.hpp"

namespace iox {
namespace dds{

///
/// @class Channel
/// @brief A data structure representing a channel between Iceoryx and DDS.
///
/// The class couples related iceoryx and dds entities that communicate with eachother to form the communication
/// channel.
/// For example: An Iceoryx subscriber and its corresponding DDS data writer, which communicate eachother to form
///              an outbound communication channel.
///
/// The structure holds pointers to the instances of the coupled entities.
/// The entites can be created and managed externally, in which case the structure only serves as a means of coupling
/// the two.
/// This can be achieved by simply calling the constructor with pointers to the instances.
///
/// Alternatively, the class can manage the entities internally in a static object pool, automatically
/// cleaning them up when the channel is discarded.
/// This can be achieved via the Channel::create method.
///
template <typename IoxInterface, typename DDSInterface>
class Channel
{
    using IoxInterfacePtr = std::shared_ptr<IoxInterface>;
    using IoxInterfacePool = iox::cxx::ObjectPool<IoxInterface, MAX_CHANNEL_NUMBER>;
    using DDSInterfacePtr = std::shared_ptr<DDSInterface>;
    using DDSInterfacePool = iox::cxx::ObjectPool<DDSInterface, MAX_CHANNEL_NUMBER>;

public:

    Channel(const iox::capro::ServiceDescription& service,
            const IoxInterfacePtr ioxInterface,
            const DDSInterfacePtr ddsInterface) noexcept;

    static Channel create(const iox::capro::ServiceDescription& service) noexcept;

    iox::capro::ServiceDescription getService() const noexcept;
    IoxInterfacePtr getIceoryInterface() const noexcept;
    DDSInterfacePtr getDDSInterface() const noexcept;

private:

    static IoxInterfacePool s_ioxInterfacePool;
    static DDSInterfacePool s_ddsInterfacePool;

    iox::capro::ServiceDescription m_service;
    IoxInterfacePtr m_ioxInterface;
    DDSInterfacePtr m_ddsInterface;

};

} // namespace dds
} // namespace iox

