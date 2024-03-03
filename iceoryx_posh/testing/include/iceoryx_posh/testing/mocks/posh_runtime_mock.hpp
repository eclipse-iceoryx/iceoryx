// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_MOCKS_POSH_RUNTIME_MOCK_HPP
#define IOX_POSH_MOCKS_POSH_RUNTIME_MOCK_HPP

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/assertions.hpp"

#include <gmock/gmock.h>

using namespace ::testing;

class PoshRuntimeMock : public iox::runtime::PoshRuntime
{
  public:
    static std::unique_ptr<PoshRuntimeMock> create(const iox::RuntimeName_t& name)
    {
        auto& runtime = mockRuntime();
        IOX_ENFORCE(!runtime.has_value(), "Using multiple PoshRuntimeMock in parallel is not supported!");
        IOX_ENFORCE(PoshRuntime::getRuntimeFactory() == PoshRuntime::defaultRuntimeFactory,
                    "The PoshRuntimeMock can only be used in combination with the "
                    "PoshRuntime::defaultRuntimeFactory! Someone else already switched the factory!");

        runtime = new PoshRuntimeMock(name);
        PoshRuntime::setRuntimeFactory(mockRuntimeFactory);
        return std::unique_ptr<PoshRuntimeMock>(runtime.value());
    }

    ~PoshRuntimeMock()
    {
        PoshRuntime::setRuntimeFactory(PoshRuntime::defaultRuntimeFactory);
        mockRuntime().reset();
    }

    MOCK_METHOD(iox::PublisherPortUserType::MemberType_t*,
                getMiddlewarePublisher,
                (const iox::capro::ServiceDescription&,
                 const iox::popo::PublisherOptions&,
                 const iox::runtime::PortConfigInfo&),
                (noexcept, override));
    MOCK_METHOD(iox::SubscriberPortUserType::MemberType_t*,
                getMiddlewareSubscriber,
                (const iox::capro::ServiceDescription&,
                 const iox::popo::SubscriberOptions&,
                 const iox::runtime::PortConfigInfo&),
                (noexcept, override));
    MOCK_METHOD(iox::popo::ClientPortUser::MemberType_t*,
                getMiddlewareClient,
                (const iox::capro::ServiceDescription&,
                 const iox::popo::ClientOptions&,
                 const iox::runtime::PortConfigInfo&),
                (noexcept, override));
    MOCK_METHOD(iox::popo::ServerPortUser::MemberType_t*,
                getMiddlewareServer,
                (const iox::capro::ServiceDescription&,
                 const iox::popo::ServerOptions&,
                 const iox::runtime::PortConfigInfo&),
                (noexcept, override));
    MOCK_METHOD(iox::popo::InterfacePortData*,
                getMiddlewareInterface,
                (const iox::capro::Interfaces, const iox::NodeName_t&),
                (noexcept, override));
    MOCK_METHOD(iox::popo::ConditionVariableData*, getMiddlewareConditionVariable, (), (noexcept, override));
    MOCK_METHOD(bool,
                sendRequestToRouDi,
                (const iox::runtime::IpcMessage&, iox::runtime::IpcMessage&),
                (noexcept, override));

  private:
    PoshRuntimeMock(const iox::RuntimeName_t& name)
        : iox::runtime::PoshRuntime(iox::optional<const iox::RuntimeName_t*>({&name}))
    {
    }

    static PoshRuntime& mockRuntimeFactory(iox::optional<const iox::RuntimeName_t*> name) noexcept
    {
        auto& runtime = mockRuntime();
        IOX_ENFORCE(!name.has_value(), "PoshRuntime::initRuntime must not be used with a PoshRuntimeMock!");
        IOX_ENFORCE(runtime.has_value(), "This should never happen! If you see this, something went horribly wrong!");
        return *runtime.value();
    }

    static iox::optional<PoshRuntimeMock*>& mockRuntime()
    {
        static iox::optional<PoshRuntimeMock*> runtime = iox::nullopt;
        return runtime;
    }
};

#endif // IOX_POSH_MOCKS_CHUNK_MOCK_HPP
