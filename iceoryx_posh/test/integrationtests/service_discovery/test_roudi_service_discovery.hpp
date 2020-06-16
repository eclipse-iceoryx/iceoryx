#ifndef IOX_POSH_SERVICE_DISCOVERY_TEST_ROUDI_SERVICE_DISCOVERY_HPP
#define IOX_POSH_SERVICE_DISCOVERY_TEST_ROUDI_SERVICE_DISCOVERY_HPP

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "roudi_gtest.hpp"

using IdString = iox::capro::IdString;
using InstanceContainer = iox::runtime::InstanceContainer;

template <class T, uint64_t Capacity>
class vector_init_list : public iox::cxx::vector<T, Capacity>
{
  public:
    vector_init_list(std::initializer_list<T> l)
    {
        for (auto& i : l)
        {
            iox::cxx::vector<T, Capacity>::push_back(i);
        }
    }
};

class RouDiServiceDiscoveryTest : public RouDi_GTest
{
  protected:
    void InitContainer(InstanceContainer& dest, std::vector<std::string> src)
    {
        dest.clear();
        for (size_t i = 0; i < src.size(); i++)
        {
            dest.push_back(IdString(iox::cxx::TruncateToCapacity, src[i]));
        }
    }

    static void ContainersEq(const InstanceContainer& cont1, const InstanceContainer& cont2)
    {
        ASSERT_THAT(cont1.size(), Eq(cont2.size()));
        for (size_t i = 0; i < cont1.size(); i++)
        {
            ASSERT_THAT(cont1[i], Eq(cont2[i]));
        }
    }
};

#endif // IOX_POSH_SERVICE_DISCOVERY_TEST_ROUDI_SERVICE_DISCOVERY_HPP
