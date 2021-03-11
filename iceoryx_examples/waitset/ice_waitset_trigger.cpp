// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include <iostream>
#include <thread>

// The two events the MyTriggerClass offers
enum class MyTriggerClassEvents
{
    PERFORMED_ACTION,
    ACTIVATE
};

// Triggerable class which has two events an both events can be
// attached to a WaitSet.
class MyTriggerClass
{
  public:
    MyTriggerClass() = default;
    ~MyTriggerClass() = default;

    // IMPORTANT: For now the WaitSet does not support that the origin is moved
    //            or copied. To support that we have to inform the waitset about
    //            our new origin, otherwise the WaitSet would end up in the wrong
    //            memory location when it calls the `hasTriggerCallback` with the
    //            old origin (already moved) origin pointer. The same goes for
    //            the resetCallback which is used when the WaitSet goes out of scope
    //            and is pointing also to the old origin.
    MyTriggerClass(const MyTriggerClass&) = delete;
    MyTriggerClass(MyTriggerClass&&) = delete;
    MyTriggerClass& operator=(const MyTriggerClass&) = delete;
    MyTriggerClass& operator=(MyTriggerClass&&) = delete;

    // When you call this method you will trigger the ACTIVATE event
    void activate(const uint64_t activationCode) noexcept
    {
        m_activationCode = activationCode;
        m_isActivated = true;
        m_activateTrigger.trigger();
    }

    // Calling this method will trigger the PERFORMED_ACTION event
    void performAction() noexcept
    {
        m_hasPerformedAction = true;
        m_onActionTrigger.trigger();
    }

    uint64_t getActivationCode() const noexcept
    {
        return m_activationCode;
    }

    // required by the m_onActionTrigger to ask the class if it was triggered
    bool hasPerformedAction() const noexcept
    {
        return m_hasPerformedAction;
    }

    // required by the m_activateTrigger to ask the class if it was triggered
    bool isActivated() const noexcept
    {
        return m_isActivated;
    }

    // reset PERFORMED_ACTION and ACTIVATE event
    void reset(const MyTriggerClassEvents event) noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORMED_ACTION:
            m_hasPerformedAction = false;
            break;
        case MyTriggerClassEvents::ACTIVATE:
            m_isActivated = false;
            break;
        }
    }

    static void callOnAction(MyTriggerClass* const triggerClassPtr)
    {
        // Ignore unused variable warning
        (void)triggerClassPtr;
        std::cout << "action performed" << std::endl;
    }

    friend iox::popo::EventAttorney;

  private:
    /// @brief Only usable by the WaitSet, not for public use
    // This method attaches an event of the class to a waitset.
    // The event is choosen by the event parameter. Additionally, you can
    // set a eventId to group multiple instances and a custom callback.
    void enableEvent(iox::popo::TriggerHandle&& triggerHandle, const MyTriggerClassEvents event) noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORMED_ACTION:
            m_onActionTrigger = std::move(triggerHandle);
            break;
        case MyTriggerClassEvents::ACTIVATE:
            m_activateTrigger = std::move(triggerHandle);
            break;
        }
    }

    /// @brief Only usable by the WaitSet, not for public use
    // we offer the waitset a method to invalidate trigger if it goes
    // out of scope
    void invalidateTrigger(const uint64_t uniqueTriggerId)
    {
        if (m_onActionTrigger.getUniqueId() == uniqueTriggerId)
        {
            m_onActionTrigger.invalidate();
        }
        else if (m_activateTrigger.getUniqueId() == uniqueTriggerId)
        {
            m_activateTrigger.invalidate();
        }
    }

    void disableEvent(const MyTriggerClassEvents event) noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORMED_ACTION:
            m_onActionTrigger.reset();
            break;
        case MyTriggerClassEvents::ACTIVATE:
            m_activateTrigger.reset();
            break;
        }
    }

    /// @brief Only usable by the WaitSet, not for public use
    iox::popo::WaitSetHasTriggeredCallback
    getHasTriggeredCallbackForEvent(const MyTriggerClassEvents event) const noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORMED_ACTION:
            return {*this, &MyTriggerClass::hasPerformedAction};
        case MyTriggerClassEvents::ACTIVATE:
            return {*this, &MyTriggerClass::isActivated};
        }
        return {};
    }

  private:
    uint64_t m_activationCode = 0U;
    bool m_hasPerformedAction = false;
    bool m_isActivated = false;

    iox::popo::TriggerHandle m_onActionTrigger;
    iox::popo::TriggerHandle m_activateTrigger;
};

iox::cxx::optional<iox::popo::WaitSet<>> waitset;
iox::cxx::optional<MyTriggerClass> triggerClass;

constexpr uint64_t ACTIVATE_ID = 0U;
constexpr uint64_t ACTION_ID = 1U;

void callOnActivate(MyTriggerClass* const triggerClassPtr)
{
    std::cout << "activated with code: " << triggerClassPtr->getActivationCode() << std::endl;
}

// The global event loop. It will create an infinite loop and
// will work on the incoming events.
void eventLoop()
{
    while (true)
    {
        auto eventVector = waitset->wait();
        for (auto& event : eventVector)
        {
            if (event->getEventId() == ACTIVATE_ID)
            {
                // reset MyTriggerClass instance state
                event->getOrigin<MyTriggerClass>()->reset(MyTriggerClassEvents::ACTIVATE);
                // call the callback attached to the trigger
                (*event)();
            }
            else if (event->getEventId() == ACTION_ID)
            {
                // reset MyTriggerClass instance state
                event->getOrigin<MyTriggerClass>()->reset(MyTriggerClassEvents::PERFORMED_ACTION);
                // call the callback attached to the trigger
                (*event)();
            }
        }
    }
}

int main()
{
    iox::runtime::PoshRuntime::initRuntime("iox-ex-waitset-trigger");

    // we create a waitset and a triggerClass instance inside of the two
    // global optional's
    waitset.emplace();
    triggerClass.emplace();

    // attach both events to a waitset and assign a callback
    waitset->attachEvent(*triggerClass, MyTriggerClassEvents::ACTIVATE, ACTIVATE_ID, &callOnActivate);
    waitset->attachEvent(
        *triggerClass, MyTriggerClassEvents::PERFORMED_ACTION, ACTION_ID, &MyTriggerClass::callOnAction);

    // start the event loop which is handling the events
    std::thread eventLoopThread(eventLoop);

    // start a thread which will trigger a event every second
    std::thread triggerThread([&] {
        uint64_t activationCode = 1U;
        for (auto i = 0U; i < 10; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->activate(activationCode++);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->performAction();
        }
    });

    triggerThread.join();
    eventLoopThread.join();
    return (EXIT_SUCCESS);
}
