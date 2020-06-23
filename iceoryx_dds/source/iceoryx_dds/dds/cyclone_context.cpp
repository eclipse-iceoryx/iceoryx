#include "iceoryx_dds/dds/cyclone_context.hpp"

::dds::domain::DomainParticipant& iox::dds::CycloneContext::getParticipant() noexcept
{
    static auto participant = ::dds::domain::DomainParticipant(org::eclipse::cyclonedds::domain::default_id());
    return participant;
}
