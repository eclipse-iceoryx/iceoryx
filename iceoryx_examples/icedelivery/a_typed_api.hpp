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

#pragma once

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

#include "topic_data.hpp"

#include <functional>
#include <memory>

template <typename TopicType>
using SampleDeleter = std::function<void(TopicType*)>;

template <typename TopicType>
using SamplePtr = std::unique_ptr<TopicType, SampleDeleter<TopicType>>;

/// @brief A typed publisher that takes the topic type as template argument
/// A RAII pattern is used with offering the the publisher in c'tor and stop offering in d'tor
/// This class has the limitation that the topic type is a fixed size data structure.
/// I.e. we can get the size of memory to allocate with the sizeof() operation.
/// So the topic type is not allowed to use heap-based members like a std::vector with default allocator
/// allocate() returns a unique_ptr to a sample. This must be moved to the publish call for sending
/// If the unique_ptr goes out of scope without publishing, the provided custom deleter frees the memory chunk
template <typename TopicType>
class TypedPublisher
{
  public:
    TypedPublisher(const iox::capro::ServiceDescription& id)
        : m_publisher(id)
    {
        m_publisher.offer();
    }

    ~TypedPublisher()
    {
        m_publisher.stopOffer();
    }

    SamplePtr<TopicType> allocate()
    {
        // allocate a chunk with the size of the topic
        auto sample = static_cast<TopicType*>(m_publisher.allocateChunk(sizeof(TopicType)));

        if (nullptr != sample)
        {
            // call the c'tor with a placement new
            sample = new (sample) TopicType;

            // return a unique_ptr which holds the sample
            return SamplePtr<TopicType>(sample, [this](TopicType* chunk) { this->m_publisher.freeChunk(chunk); });
        }
        else
        {
            // no more memory in the middleware :-(
            return SamplePtr<TopicType>(nullptr, [this](TopicType* chunk) { this->m_publisher.freeChunk(chunk); });
        }
    }

    void publish(SamplePtr<TopicType> samplePtr)
    {
        // only send if the SamplePtr holds a sample
        if (samplePtr)
        {
            // get the sample in the SamplePtr
            auto sample = samplePtr.release();

            // pass the sample to the middleware
            m_publisher.sendChunk(sample);
        }
    }

  private:
    iox::popo::Publisher m_publisher;
};


template <typename TopicType>
using OnReceiveCallback = std::function<void(const TopicType&)>;

/// @brief A typed subsciber that takes the topic type as template argument
/// Additionally a callback to process a received sample is provided with the c'tor
/// A RAII pattern is used for subscribing in the c'tor and unsubscribing in the d'tor
/// this class will call the callback for each received sample. After the callback execution the sample is released
template <typename TopicType>
class TypedSubscriber
{
  public:
    TypedSubscriber(const iox::capro::ServiceDescription& id, OnReceiveCallback<TopicType> callback)
        : m_subscriber(id)
        , m_callback(callback)
    {
        m_subscriber.setReceiveHandler(std::bind(&TypedSubscriber::receiveHandler, this));
        m_subscriber.subscribe();
    }

    ~TypedSubscriber()
    {
        m_subscriber.unsubscribe();
        m_subscriber.unsetReceiveHandler();
    }


  private:
    void receiveHandler()
    {
        const void* chunk = nullptr;

        // get all the chunks the FiFo holds. Maybe there are several ones if the publisher produces faster than the
        // subscriber can process
        while (m_subscriber.getChunk(&chunk))
        {
            auto sample = static_cast<const TopicType*>(chunk);

            // call the provided callback
            m_callback(*sample);

            // release the chunk
            m_subscriber.releaseChunk(chunk);
        }
    }

    iox::popo::Subscriber m_subscriber;
    OnReceiveCallback<TopicType> m_callback;
};