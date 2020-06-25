#ifndef CYCLONE_CONTEXT_HPP
#define CYCLONE_CONTEXT_HPP

#include <dds/dds.hpp>

namespace iox
{
namespace dds
{
class CycloneContext
{
  public:
    static ::dds::domain::DomainParticipant& getParticipant() noexcept;
};

} // namespace dds
} // namespace iox

#endif // CYCLONE_CONTEXT_HPP
