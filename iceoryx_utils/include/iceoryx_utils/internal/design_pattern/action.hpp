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

namespace DesignPattern
{
// base class for void(Arg) calls - known as commands
///@todo: variadic arguments and perfect forwarding
///@todo variant with return value and other nice features (in essence, a generic functor in contrast to lambdas)
template <typename Arg>
class Command
{
  public:
    void operator()(Arg& arg)
    {
        exec(arg);
    }

    virtual ~Command()
    {
    }

  protected:
    virtual void exec(Arg& arg)
    {
        (void)arg;
    }
};

// base class for void(void) calls - also known as actions
template <>
class Command<void>
{
  public:
    void operator()()
    {
        exec();
    }

    virtual ~Command()
    {
    }

  protected:
    virtual void exec(){};
};

using Action = Command<void>;
}
