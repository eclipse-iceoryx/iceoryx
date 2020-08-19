// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_binding_c/posh_runtime.h"
#include "iceoryx_binding_c/subscriber.h"

void receiving()
{
}

int main()
{
    PoshRuntime_getInstance("/iox-c-publisher");

    struct SubscriberPortData* subscriber = Subscriber_new();
    Subscriber_subscribe(subscriber, 10);
    Subscriber_delete(subscriber);
    return 0;
}
