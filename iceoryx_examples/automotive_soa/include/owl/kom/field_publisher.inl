// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_INL
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_INL

#include "owl/kom/field_publisher.hpp"

namespace owl
{
namespace kom
{
template <typename T>
inline FieldPublisher<T>::FieldPublisher(const ServiceIdentifier& service,
                                         const InstanceIdentifier& instance,
                                         const FieldIdentifier& field,
                                         const FieldType& fieldValue) noexcept
    : m_publisher({service, instance, field}, {HISTORY_CAPACITY})
    , m_server({service, instance, field})
    , m_latestValue(fieldValue)
{
    // publisher is automatically offered
    Update(m_latestValue);

    //! [FieldPublisher attach]
    m_listener
        .attachEvent(m_server,
                     iox::popo::ServerEvent::REQUEST_RECEIVED,
                     iox::popo::createNotificationCallback(onRequestReceived, *this))
        .expect("Unable to attach server!");
    //! [FieldPublisher attach]
}

template <typename T>
inline FieldPublisher<T>::~FieldPublisher() noexcept
{
    m_listener.detachEvent(m_server, iox::popo::ServerEvent::REQUEST_RECEIVED);
    m_publisher.stopOffer();
}

template <typename T>
inline bool FieldPublisher<T>::Update(const FieldType& userField) noexcept
{
    auto maybeField = m_publisher.loan();

    if (maybeField.has_error())
    {
        std::cerr << "Error occured during allocation, couldn't send sample!" << std::endl;
        return false;
    }

    auto& field = maybeField.value();
    *(field.get()) = userField;
    m_latestValue = userField;
    field.publish();
    return true;
}

template <typename T>
inline void FieldPublisher<T>::RegisterGetHandler(iox::cxx::function<void()>) noexcept
{
    std::cerr << "'RegisterGetHandler' not implemented." << std::endl;
}

template <typename T>
inline void FieldPublisher<T>::RegisterSetHandler(iox::cxx::function<void()>) noexcept
{
    std::cerr << "'RegisterSetHandler' not implemented." << std::endl;
}

//! [FieldPublisher callback]
template <typename T>
inline void FieldPublisher<T>::onRequestReceived(iox::popo::Server<iox::cxx::optional<FieldType>, FieldType>* server,
                                                 FieldPublisher<FieldType>* self) noexcept
{
    if (server == nullptr || self == nullptr)
    {
        std::cerr << "Callback was invoked with server or self being a nullptr!" << std::endl;
        return;
    }

    while (server->take().and_then([&](const auto& request) {
        server->loan(request)
            .and_then([&](auto& response) {
                if (request->has_value())
                {
                    self->m_latestValue = request->value();
                }
                *response = self->m_latestValue;
                response.send().or_else(
                    [&](auto& error) { std::cerr << "Could not send response! Error: " << error << std::endl; });
            })
            .or_else([](auto& error) { std::cerr << "Could not allocate response! Error: " << error << std::endl; });
    }))
    {
    }
}
//! [FieldPublisher callback]
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_INL
