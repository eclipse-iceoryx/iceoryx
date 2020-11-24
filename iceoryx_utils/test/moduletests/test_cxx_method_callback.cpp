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

#include "test.hpp"

#include "iceoryx_utils/cxx/method_callback.hpp"

using namespace ::testing;
using namespace iox;
using namespace iox::cxx;


class MethodCallback_test : public Test
{
  public:
    class TestClass
    {
      public:
        void voidVoidMethod()
        {
        }

        void constVoidVoidMethod() const
        {
        }
    };
    TestClass m_testClass;
};

TEST_F(MethodCallback_test, NullptrClassPtrInCtorLeadsToInvalidMethodCallback)
{
    MethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::voidVoidMethod);
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(MethodCallback_test, NullptrClassPtrInCtorLeadsToInvalidConstMethodCallback)
{
    ConstMethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::constVoidVoidMethod);
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(MethodCallback_test, NonNullptrClassPtrInCtorLeadsToValidMethodCallback)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(MethodCallback_test, NonNullptrClassPtrInCtorLeadsToValidConstMethodCallback)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(MethodCallback_test, AssignNonNullptrClassPtrLeadsToValidMethodCallback)
{
    MethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::voidVoidMethod);
    sut.setClassPtr(&m_testClass);
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(MethodCallback_test, AssignNonNullptrClassPtrLeadsToValidConstMethodCallback)
{
    ConstMethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::constVoidVoidMethod);
    sut.setClassPtr(&m_testClass);
    EXPECT_TRUE(static_cast<bool>(sut));
}

TEST_F(MethodCallback_test, AssignNullptrClassPtrLeadsToInvalidMethodCallback)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    sut.setClassPtr(static_cast<TestClass*>(nullptr));
    EXPECT_FALSE(static_cast<bool>(sut));
}

TEST_F(MethodCallback_test, AssignNullptrClassPtrLeadsToInvalidConstMethodCallback)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    sut.setClassPtr(static_cast<TestClass*>(nullptr));
    EXPECT_FALSE(static_cast<bool>(sut));
}

