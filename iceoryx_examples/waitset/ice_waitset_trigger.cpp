// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include <iostream>
#include <thread>

enum class MyTriggerClassEvents
{
    PERFORMED_ACTION,
    ACTIVATE
};

class MyTriggerClass
{
  public:
    void activate(const int activationCode) noexcept
    {
        m_activationCode = activationCode;
        m_isActivated = true;
        m_activateTrigger.trigger();
    }

    void performAcion() noexcept
    {
        m_hasPerformedAction = true;
        m_actionTrigger.trigger();
    }

    int getActivationCode() const noexcept
    {
        return m_activationCode;
    }

    bool hasPerformedAction() const noexcept
    {
        return m_hasPerformedAction;
    }

    bool isActivated() const noexcept
    {
        return m_isActivated;
    }

    void reset() noexcept
    {
        m_hasPerformedAction = false;
        m_isActivated = false;
    }

    iox::cxx::expected<iox::popo::WaitSetError>
    attachToWaitset(iox::popo::WaitSet& waitset,
                    const MyTriggerClassEvents event,
                    const uint64_t triggerId,
                    const iox::popo::Trigger::Callback<MyTriggerClass> callback) noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORMED_ACTION:
        {
            return waitset
                .acquireTrigger(this,
                                {this, &MyTriggerClass::hasPerformedAction},
                                {this, &MyTriggerClass::unsetTrigger},
                                triggerId,
                                callback)
                .and_then([this](iox::popo::Trigger& trigger) { m_actionTrigger = std::move(trigger); });
        }
        case MyTriggerClassEvents::ACTIVATE:
        {
            return waitset
                .acquireTrigger(this,
                                {this, &MyTriggerClass::isActivated},
                                {this, &MyTriggerClass::unsetTrigger},
                                triggerId,
                                callback)
                .and_then([this](iox::popo::Trigger& trigger) { m_activateTrigger = std::move(trigger); });
        }
        }
    }

    void unsetTrigger(const iox::popo::Trigger& trigger)
    {
        if (trigger == m_actionTrigger)
        {
            m_actionTrigger.reset();
        }
        else if (trigger == m_activateTrigger)
        {
            m_activateTrigger.reset();
        }
    }

    static void callOnAction(MyTriggerClass* const triggerClassPtr)
    {
        std::cout << "action performed" << std::endl;
    }

  private:
    int m_activationCode = 0;
    bool m_hasPerformedAction = false;
    bool m_isActivated = false;

    iox::popo::Trigger m_actionTrigger;
    iox::popo::Trigger m_activateTrigger;
};

iox::cxx::optional<iox::popo::WaitSet> waitset;
iox::cxx::optional<MyTriggerClass> triggerClass;

constexpr uint64_t ACTIVATE_ID = 0;
constexpr uint64_t ACTION_ID = 1;

void callOnActivate(MyTriggerClass* const triggerClassPtr)
{
    std::cout << "activated with code: " << triggerClassPtr->getActivationCode() << std::endl;
}

void backgroundThread()
{
    while (true)
    {
        auto triggerStateVector = waitset->wait();
        for (auto& triggerState : triggerStateVector)
        {
            if (triggerState.getTriggerId() == ACTIVATE_ID)
            {
                triggerState();
                triggerState.getOrigin<MyTriggerClass>()->reset();
            }
            else if (triggerState.getTriggerId() == ACTION_ID)
            {
                triggerState();
                triggerState.getOrigin<MyTriggerClass>()->reset();
            }
        }
    }
}

int main()
{
    iox::runtime::PoshRuntime::getInstance("/iox-ex-waitset-trigger");

    waitset.emplace();
    triggerClass.emplace();

    triggerClass->attachToWaitset(*waitset, MyTriggerClassEvents::ACTIVATE, ACTIVATE_ID, callOnActivate);
    triggerClass->attachToWaitset(
        *waitset, MyTriggerClassEvents::PERFORMED_ACTION, ACTION_ID, MyTriggerClass::callOnAction);

    std::thread t(backgroundThread);
    std::thread triggerThread([&] {
        int activationCode = 1;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->activate(activationCode++);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->performAcion();
        }
    });

    triggerThread.join();
    t.join();
    return (EXIT_SUCCESS);
}
