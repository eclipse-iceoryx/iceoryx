// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "roudi_gtest.hpp"
#include "test_roudi_service_discovery.hpp"

constexpr uint32_t INTER_OP_WAIT_FOR_SERVICE_DISCOVERY = 500;
const std::string AnyInstanceString = iox::capro::AnyInstanceString;
constexpr char TEST_SERVICE[] = "service1";
const iox::capro::ServiceDescription singleServiceSingleInstance = {TEST_SERVICE, "instance1"};
const IdString startFindServiceInputParam = {TEST_SERVICE};

class RoudiStartFindService_test : public RouDiServiceDiscoveryTest
{
  public:
    void SetUp()
    {
        this->SetInterOpWaitingTime(std::chrono::milliseconds(INTER_OP_WAIT_FOR_SERVICE_DISCOVERY));
        clear();
    }

    void TearDown()
    {
    }


    static void FindServiceHandler(InstanceContainer instanceContainer, FindServiceHandle handle)
    {
        m_currentInstances = instanceContainer;
        m_handle = handle;
        m_FindServiceHandlerCallCount++;
    }

    static void clear()
    {
        m_currentInstances = {};
        m_handle = -1;
        m_FindServiceHandlerCallCount = 0;
    }

    iox::runtime::PoshRuntime* m_senderRuntime{&iox::runtime::PoshRuntime::getInstance("/sender")};
    iox::runtime::PoshRuntime* m_receiverRuntime{&iox::runtime::PoshRuntime::getInstance("/receiver")};

    static InstanceContainer m_currentInstances;
    static FindServiceHandle m_handle;
    static uint32_t m_FindServiceHandlerCallCount;
};

InstanceContainer RoudiStartFindService_test::m_currentInstances;
iox::runtime::FindServiceHandle RoudiStartFindService_test::m_handle;
uint32_t RoudiStartFindService_test::m_FindServiceHandlerCallCount = 0;

TEST_F(RoudiStartFindService_test, SingleServiceSingleInstance)
{
    InstanceContainer instanceContainerExp;
    InitContainer(instanceContainerExp, {"instance1"});

    auto l_handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();

    m_senderRuntime->offerService({"service1", "instance1"});
    InterOpWait();
    EXPECT_THAT(m_handle, Eq(l_handle));
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));
}

TEST_F(RoudiStartFindService_test, SingleServiceSingleInstance_OfferStopOfferOfferSequence)
{
    InstanceContainer instanceContainerExp;
    InitContainer(instanceContainerExp, {"instance1"});

    auto l_handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();

    m_senderRuntime->offerService({"service1", "instance1"});
    InterOpWait();
    EXPECT_THAT(m_handle, l_handle);
    ContainersEq(m_currentInstances, instanceContainerExp);

    m_senderRuntime->stopOfferService({"service1", "instance1"});
    InterOpWait();
    ContainersEq(m_currentInstances, {});

    m_senderRuntime->offerService({"service1", "instance1"});
    InterOpWait();
    ContainersEq(m_currentInstances, instanceContainerExp);
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(3u));
}

TEST_F(RoudiStartFindService_test, stopFindService)
{
    FindServiceHandle l_handle;
    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();

    m_senderRuntime->offerService({"service1", "instance1"});
    InterOpWait();

    m_receiverRuntime->stopFindService(l_handle);
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));

    // Any change in service state , after calling stopFindService(), wont lead to firing of FindServiceHandler
    m_senderRuntime->stopOfferService({"service1", "instance1"});
    InterOpWait();
    m_senderRuntime->offerService({"service1", "instance1"});
    InterOpWait();
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));
}


const std::vector<iox::capro::ServiceDescription> singleServiceMultiInstance = {
    {"service1", "instance1"}, {"service1", "instance2"}, {"service1", "instance3"}};

TEST_F(RoudiStartFindService_test, SingleServiceMultiInstanceSimultaneousOfferStopOfferOffer)
{
    auto l_handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();

    InstanceContainer expectedInstances(singleServiceMultiInstance.size());
    std::transform(singleServiceMultiInstance.begin(),
                   singleServiceMultiInstance.end(),
                   expectedInstances.begin(),
                   [](iox::capro::ServiceDescription s) { return s.getInstanceIDString(); });

    for (auto const& it : singleServiceMultiInstance)
    {
        m_senderRuntime->offerService(it);
    }
    InterOpWait();
    EXPECT_THAT(m_handle, l_handle);
    ContainersEq(m_currentInstances, expectedInstances);

    for (auto const& it : singleServiceMultiInstance)
    {
        m_senderRuntime->stopOfferService(it);
    }
    InterOpWait();
    ContainersEq(m_currentInstances, {});

    for (auto const& it : singleServiceMultiInstance)
    {
        m_senderRuntime->offerService(it);
    }
    InterOpWait();

    ContainersEq(m_currentInstances, expectedInstances);
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(3u));
}


TEST_F(RoudiStartFindService_test, SingleServiceMultiInstanceSequentialOfferStopOfferOffer)
{
    m_receiverRuntime->startFindService(FindServiceHandler, {"service1"});

    int expectedInstanceCount = 0;
    InstanceContainer expectedInstances;
    for (auto const& it : singleServiceMultiInstance)
    {
        expectedInstances.push_back(it.getInstanceIDString());
        m_senderRuntime->offerService(it);
        InterOpWait();
        ASSERT_THAT(m_currentInstances.size(), Eq(++expectedInstanceCount));
        ContainersEq(m_currentInstances, expectedInstances);
    }

    for (auto const& it : singleServiceMultiInstance)
    {
        expectedInstances.erase(expectedInstances.begin());
        m_senderRuntime->stopOfferService(it);
        InterOpWait();
        ASSERT_THAT(m_currentInstances.size(), Eq(--expectedInstanceCount));
        ContainersEq(m_currentInstances, expectedInstances);
    }

    for (auto const& it : singleServiceMultiInstance)
    {
        expectedInstances.push_back(it.getInstanceIDString());
        m_senderRuntime->offerService(it);
        InterOpWait();
        ASSERT_THAT(m_currentInstances.size(), Eq(++expectedInstanceCount));
        ContainersEq(m_currentInstances, expectedInstances);
    }
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq((3 * 3)));
}

std::vector<iox::capro::ServiceDescription> multiServiceSingleInstance = {
    {"service1", "instance1"}, {"service2", "instance2"}, {"service3", "instance3"}};

struct serviceDetails
{
    InstanceContainer instances;
    FindServiceHandle handle;
};

TEST_F(RoudiStartFindService_test, MultiServiceSingleInstanceSimultaneousOfferStopOfferOffer)
{
    int findServiceHandlerCallCount = 0;
    std::map<IdString, serviceDetails> currentInstance;

    for (const auto& element : multiServiceSingleInstance)
    {
        m_receiverRuntime->startFindService(
            [&](InstanceContainer instanceContainer, FindServiceHandle handle) {
                findServiceHandlerCallCount++;
                serviceDetails details = {InstanceContainer(instanceContainer), handle};
                currentInstance[element.getServiceIDString()] = details;
                /// @todo: Assert handle here
            },
            element.getServiceIDString());
    }

    for (const auto& element : multiServiceSingleInstance)
    {
        m_senderRuntime->offerService(element);
    }
    InterOpWait();

    for (const auto& element : multiServiceSingleInstance)
    {
        InstanceContainer expectedInstances;
        InitContainer(expectedInstances, {element.getInstanceIDString()});

        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(it->second.instances, expectedInstances);
    }

    for (const auto& element : multiServiceSingleInstance)
    {
        m_senderRuntime->stopOfferService(element);
    }
    InterOpWait();

    for (const auto& element : multiServiceSingleInstance)
    {
        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(it->second.instances, {});
    }

    for (const auto& element : multiServiceSingleInstance)
    {
        m_senderRuntime->offerService(element);
    }
    InterOpWait();

    for (const auto& element : multiServiceSingleInstance)
    {
        InstanceContainer expectedInstances;
        InitContainer(expectedInstances, {element.getInstanceIDString()});

        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(it->second.instances, expectedInstances);
    }
    ASSERT_THAT(findServiceHandlerCallCount, Eq(3 * 3));
}

TEST_F(RoudiStartFindService_test, MultiServiceSingleInstanceSequentialOfferStopOfferOffer)
{
    int findServiceHandlerCallCount = 0;
    std::map<IdString, serviceDetails> currentInstance;

    // Default value is 200ms
    SetInterOpWaitingTime(std::chrono::milliseconds(400));

    for (const auto& element : multiServiceSingleInstance)
    {
        m_receiverRuntime->startFindService(
            [&](InstanceContainer instanceContainer, FindServiceHandle handle) {
                findServiceHandlerCallCount++;
                serviceDetails details = {InstanceContainer(instanceContainer), handle};
                currentInstance[element.getServiceIDString()] = details;
            },
            element.getServiceIDString());
    }

    for (auto const& element : multiServiceSingleInstance)
    {
        InstanceContainer expectedInstances;
        InitContainer(expectedInstances, {element.getInstanceIDString()});

        m_senderRuntime->offerService(element);
        InterOpWait();
        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(it->second.instances, expectedInstances);
    }

    for (auto const& element : multiServiceSingleInstance)
    {
        m_senderRuntime->stopOfferService(element);
        InterOpWait();
        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(it->second.instances, {});
    }

    for (auto const& element : multiServiceSingleInstance)
    {
        InstanceContainer expectedInstances;
        InitContainer(expectedInstances, {element.getInstanceIDString()});

        m_senderRuntime->offerService(element);
        InterOpWait();
        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(it->second.instances, expectedInstances);
    }
    ASSERT_THAT(findServiceHandlerCallCount, Eq(3 * 3));
}


/// @todo : test case doesnt work for larger values
// Doesnt work for more 150 or more
constexpr uint32_t MAX_NUMBER_OF_SERVICES = iox::MAX_START_FIND_SERVICE_CALLBACKS;
constexpr uint32_t INTER_OP_WAIT_FOR_LARGE_SERVICES = 500;

TEST_F(RoudiStartFindService_test, LargeNumberOfServices)
{
    std::vector<iox::capro::ServiceDescription> testServiceDescriptors;

    for (uint i = 0; i < MAX_NUMBER_OF_SERVICES; i++)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        testServiceDescriptors.push_back(
            {IdString(iox::cxx::TruncateToCapacity, service), IdString(iox::cxx::TruncateToCapacity, instance)});
    }

    int findServiceHandlerCallCount = 0;
    std::map<IdString, serviceDetails> currentInstance;

    for (const auto& element : testServiceDescriptors)
    {
        m_receiverRuntime->startFindService(
            [&](InstanceContainer instanceContainer, FindServiceHandle handle) {
                findServiceHandlerCallCount++;
                serviceDetails details = {InstanceContainer(instanceContainer), handle};
                currentInstance[element.getServiceIDString()] = details;
            },
            element.getServiceIDString());
    }

    for (const auto& element : testServiceDescriptors)
    {
        m_senderRuntime->offerService(element);
    }
    InterOpWait();

    for (const auto& element : testServiceDescriptors)
    {
        InstanceContainer expectedInstances;
        InitContainer(expectedInstances, {element.getInstanceIDString()});

        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(expectedInstances, it->second.instances);
    }

    for (const auto& element : testServiceDescriptors)
    {
        m_senderRuntime->stopOfferService(element);
    }
    InterOpWait();

    for (const auto& element : testServiceDescriptors)
    {
        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq({}, it->second.instances);
    }

    for (const auto& element : testServiceDescriptors)
    {
        m_senderRuntime->offerService(element);
    }
    InterOpWait();

    for (const auto& element : testServiceDescriptors)
    {
        InstanceContainer expectedInstances;
        InitContainer(expectedInstances, {element.getInstanceIDString()});

        auto it = currentInstance.find(element.getServiceIDString());
        ContainersEq(expectedInstances, it->second.instances);
    }
    EXPECT_THAT(findServiceHandlerCallCount, Eq((3 * MAX_NUMBER_OF_SERVICES)));
}


TEST_F(RoudiStartFindService_test, LargeNumberOfInstance)
{
    std::vector<iox::capro::ServiceDescription> testServiceDescriptors;

    for (uint i = 0; i < MAX_NUMBER_OF_SERVICES; i++)
    {
        std::string service = "service";
        std::string instance = "i" + std::to_string(i);
        testServiceDescriptors.push_back(
            {IdString(iox::cxx::TruncateToCapacity, service), IdString(iox::cxx::TruncateToCapacity, instance)});
    }

    m_receiverRuntime->startFindService(FindServiceHandler, {"service"});

    for (const auto& element : testServiceDescriptors)
    {
        m_senderRuntime->offerService(element);
    }
    InterOpWait();

    InstanceContainer expectedInstances;
    for (const auto& element : testServiceDescriptors)
    {
        expectedInstances.push_back(element.getInstanceIDString());
    }
    ContainersEq(m_currentInstances, expectedInstances);

    for (const auto& element : testServiceDescriptors)
    {
        m_senderRuntime->stopOfferService(element);
    }
    InterOpWait();
    ContainersEq(m_currentInstances, {});

    for (const auto& element : testServiceDescriptors)
    {
        m_senderRuntime->offerService(element);
    }
    InterOpWait();

    for (const auto& element : testServiceDescriptors)
    {
        expectedInstances.push_back(element.getInstanceIDString());
    }
    ContainersEq(m_currentInstances, expectedInstances);
    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(3u));
}


// Corner cases
/// @todo : This test fails due to deadlock in mutex
TEST_F(RoudiStartFindService_test, stopFindServiceFromCallback)
{
    m_receiverRuntime->startFindService(
        [&](InstanceContainer instanceContainer, FindServiceHandle handle) {
            // Stop find service , when any service is found
            if (instanceContainer.size() > 0)
            {
                m_receiverRuntime->stopFindService(handle);
            }
        },
        startFindServiceInputParam);

    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();
}

TEST_F(RoudiStartFindService_test, startFindServiceMultipleCalls)
{
    FindServiceHandle l_handle;

    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, startFindServiceInputParam).get_value();

    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();

    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));

    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, startFindServiceInputParam).get_value();
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));
    ASSERT_THAT(l_handle, m_handle);
}

/// startFindService is called , after services are offered
TEST_F(RoudiStartFindService_test, DeferredStartFindService)
{
    InstanceContainer expectedInstances;
    InitContainer(expectedInstances, {"instance1"});

    m_senderRuntime->offerService({"service1", "instance1"});
    m_senderRuntime->offerService({"service2", "instance1"});
    InterOpWait();

    FindServiceHandle l_handle;
    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, "service1").get_value();
    InterOpWait();
    /// FindServiceHandler() is called immediately
    EXPECT_THAT(m_currentInstances.size(), Eq(1u));
    ContainersEq(m_currentInstances, expectedInstances);
    EXPECT_THAT(m_handle, l_handle);
    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(1u));

    clear();
    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, "service2").get_value();
    InterOpWait();

    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(1u));
    EXPECT_THAT(m_currentInstances.size(), Eq(1u));
    ContainersEq(m_currentInstances, expectedInstances);
    EXPECT_THAT(m_handle, l_handle);
}

TEST_F(RoudiStartFindService_test, stopFindServiceRedudantCalls)
{
    FindServiceHandle l_handle;

    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, startFindServiceInputParam).get_value();

    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();

    m_receiverRuntime->stopFindService(l_handle);
    m_receiverRuntime->stopFindService(l_handle);
    m_receiverRuntime->stopFindService(l_handle);
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));

    // Any change in service state , after calling stopFindService(), wont lead to firing of FindServiceHandler
    m_senderRuntime->stopOfferService(singleServiceSingleInstance);
    InterOpWait();
    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));
}

TEST_F(RoudiStartFindService_test, stopFindServiceRedudantCallsWithWrongInput)
{
    FindServiceHandle l_handle;

    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, startFindServiceInputParam).get_value();

    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();

    m_receiverRuntime->stopFindService(l_handle);
    m_receiverRuntime->stopFindService(l_handle + 1);
    m_receiverRuntime->stopFindService(l_handle + 2);
    m_receiverRuntime->stopFindService(l_handle - 1);

    // Any change in service state , after calling stopFindService(), wont lead to firing of FindServiceHandler
    m_senderRuntime->stopOfferService(singleServiceSingleInstance);
    InterOpWait();
    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();
    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(1u));
}

TEST_F(RoudiStartFindService_test, stopFindServiceWrongHandle)
{
    FindServiceHandle l_handle;

    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, startFindServiceInputParam).get_value();

    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();

    m_receiverRuntime->stopFindService(l_handle + 1);
    m_receiverRuntime->stopFindService(l_handle + 2);

    // Change in service state results in firing of FindServiceHandler, because stopFindService() is not effective
    // (called with wrong handle)
    m_senderRuntime->stopOfferService(singleServiceSingleInstance);
    InterOpWait();
    m_senderRuntime->offerService(singleServiceSingleInstance);
    InterOpWait();

    ASSERT_THAT(m_FindServiceHandlerCallCount, Eq(3u));
}

TEST_F(RoudiStartFindService_test, changeInServiceStateWithoutChangeInNumberOfInstances)
{
    m_receiverRuntime->startFindService(FindServiceHandler, startFindServiceInputParam);

    m_senderRuntime->offerService({TEST_SERVICE, "instance1"});
    m_senderRuntime->offerService({TEST_SERVICE, "instance2"});
    m_senderRuntime->offerService({TEST_SERVICE, "instance3"});

    InterOpWait();
    InstanceContainer expectedInstances;
    InitContainer(expectedInstances, {"instance1", "instance2", "instance3"});
    ContainersEq(m_currentInstances, expectedInstances);

    m_senderRuntime->stopOfferService({TEST_SERVICE, "instance3"});
    m_senderRuntime->offerService({TEST_SERVICE, "instance4"});
    InterOpWait();
    InitContainer(expectedInstances, {"instance1", "instance2", "instance4"});
    ContainersEq(expectedInstances, expectedInstances);
}

TEST_F(RoudiStartFindService_test, startFindServiceUniqueId)
{
    FindServiceHandle handle;
    std::vector<FindServiceHandle> handles;

    handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();
    handles.push_back(handle);
    handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service2"}).get_value();
    handles.push_back(handle);
    handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service3"}).get_value();
    handles.push_back(handle);

    m_receiverRuntime->stopFindService(*handles.begin());
    handles.erase(handles.begin());

    handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service4"}).get_value();
    handles.push_back(handle);

    std::set<FindServiceHandle> unique_handles(handles.begin(), handles.end());

    EXPECT_THAT(handles.size(), Eq(unique_handles.size())) << "startFindService returned duplicate handles";
}

// IMP !!! This test case should not be the first test case
TEST_F(RoudiStartFindService_test, StartFindServiceHandleRollOver)
{
    auto handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();
    EXPECT_THAT(handle, Eq(0u));
}

TEST_F(RoudiStartFindService_test, InstanceContainerOverflowErrorAtBeginning)
{
    m_currentInstances.clear();
    size_t noOfInstances = (iox::MAX_NUMBER_OF_INSTANCES + 1);
    InstanceContainer instanceContainerExp;

    for (size_t i = 0; i < noOfInstances; i++)
    {
        std::string instance = "i" + std::to_string(i);
        m_senderRuntime->offerService({"s", IdString(iox::cxx::TruncateToCapacity, instance)});
        instanceContainerExp.push_back(IdString(iox::cxx::TruncateToCapacity, instance));
    }

    InterOpWait();
    auto status = m_receiverRuntime->startFindService(FindServiceHandler, {"s"});
    InterOpWait();
    EXPECT_THAT(status.has_error(), Eq(false));
    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(0u));
    /// If number of instances cant fit , then first invocation of the FindServiceHandler is called with zero instances.
    /// (In case of overflow, its not possible to compute the delta reliably in the consequent service discovery, hence
    /// all the instances are ignored)
    ContainersEq(m_currentInstances, {});
}

TEST_F(RoudiStartFindService_test, InstanceContainerOverflowErrorIntermediate)
{
    size_t noOfInstances = iox::MAX_NUMBER_OF_INSTANCES;
    InstanceContainer instanceContainerExp;

    for (size_t i = 1; i <= noOfInstances; i++)
    {
        std::string instance = "i" + std::to_string(i);
        m_senderRuntime->offerService({"s", IdString(iox::cxx::TruncateToCapacity, instance)});
        instanceContainerExp.push_back(IdString(iox::cxx::TruncateToCapacity, instance));
    }
    InterOpWait();
    m_receiverRuntime->startFindService(FindServiceHandler, {"s"});
    InterOpWait();
    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(1u));
    ContainersEq(m_currentInstances, instanceContainerExp);

    /// hard-coded to 50 ,for the sake of readability
    /// Number of instances offered is 51, which is not ignored by the middleware, as it results in instance container
    /// overflow
    m_senderRuntime->stopOfferService({"s", "i50"});
    m_senderRuntime->offerService({"s", "i51"});
    m_senderRuntime->offerService({"s", "i52"});

    InterOpWait();

    /// callback function is not fired as  maximimum limit of instance container is reached
    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(1u));

    /// make space in instance container, by removing an instance
    m_senderRuntime->stopOfferService({"s", "i51"});
    instanceContainerExp.pop_back();
    instanceContainerExp.push_back({"i52"});
    InterOpWait();

    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(2u));
    ContainersEq(m_currentInstances, instanceContainerExp);
}

TEST_F(RoudiStartFindService_test, startFindServiceFindServiceCallbackContainerOverflow)
{
    size_t noOfInstances = iox::MAX_START_FIND_SERVICE_CALLBACKS;

    for (size_t i = 0; i < noOfInstances; i++)
    {
        auto status = m_receiverRuntime->startFindService(
            FindServiceHandler, IdString(iox::cxx::TruncateToCapacity, {"service" + std::to_string(i)}));
        EXPECT_THAT(status.has_error(), Eq(false));
    }

    // There is no room for accomodating this request
    auto status = m_receiverRuntime->startFindService(FindServiceHandler, {"service_max"});
    EXPECT_THAT(status.has_error(), Eq(true));
    EXPECT_THAT(m_FindServiceHandlerCallCount, Eq(0u));
}

TEST_F(RoudiStartFindService_test, SingleServiceSingleInstance_StartStopStartFindService)
{
    InstanceContainer instanceContainerExp;
    InitContainer(instanceContainerExp, {"instance1"});

    auto l_handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();
    InterOpWait();
    m_receiverRuntime->stopFindService(l_handle);
    InterOpWait();
    l_handle = m_receiverRuntime->startFindService(FindServiceHandler, {"service1"}).get_value();
    InterOpWait();
    m_senderRuntime->offerService({"service1", "instance1"});
    InterOpWait();
    EXPECT_THAT(m_handle, l_handle);
    ContainersEq(m_currentInstances, instanceContainerExp);
}