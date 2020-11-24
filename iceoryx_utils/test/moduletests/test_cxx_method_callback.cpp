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

        void voidVoidMethod2()
        {
        }

        void constVoidVoidMethod2() const
        {
        }

        int myMethod(int a, int b)
        {
            return a + b;
        }

        int myConstMethod(int a, int b) const
        {
            return a * b;
        }
    };

    TestClass m_testClass;
    TestClass m_testClass2;
};

TEST_F(MethodCallback_test, NullptrClassPtrInCtorLeadsToInvalidMethodCallback)
{
    MethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::voidVoidMethod);
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, NullptrClassPtrInCtorLeadsToInvalidConstMethodCallback)
{
    ConstMethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::constVoidVoidMethod);
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, NonNullptrClassPtrInCtorLeadsToValidMethodCallback)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, NonNullptrClassPtrInCtorLeadsToValidConstMethodCallback)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, GetClassPointerReturnsValidConstMethodCallbackClass)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    EXPECT_EQ(sut.getClassPointer<TestClass>(), &m_testClass);
}

TEST_F(MethodCallback_test, GetClassPointerReturnsValidMethodCallbackClass)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    EXPECT_EQ(sut.getClassPointer<TestClass>(), &m_testClass);
}

TEST_F(MethodCallback_test, AssignNonNullptrClassPtrLeadsToValidMethodCallback)
{
    MethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::voidVoidMethod);
    sut.setClassPointer(&m_testClass);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, AssignNonNullptrClassPtrLeadsToValidConstMethodCallback)
{
    ConstMethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::constVoidVoidMethod);
    sut.setClassPointer(&m_testClass);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, AssignNullptrClassPtrLeadsToInvalidMethodCallback)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    sut.setClassPointer(static_cast<TestClass*>(nullptr));
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, AssignNullptrClassPtrLeadsToInvalidConstMethodCallback)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    sut.setClassPointer(static_cast<TestClass*>(nullptr));
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, TwoConstCallbacksWithSameClassAndMethodAreEqual)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    ConstMethodCallback<void> sut2(&m_testClass, &TestClass::constVoidVoidMethod);

    EXPECT_TRUE(sut == sut2);
    EXPECT_FALSE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoCallbacksWithSameClassAndMethodAreEqual)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    MethodCallback<void> sut2(&m_testClass, &TestClass::voidVoidMethod);

    EXPECT_TRUE(sut == sut2);
    EXPECT_FALSE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoConstCallbacksWithWithDifferentClassPtrAreNotEqual)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    ConstMethodCallback<void> sut2(&m_testClass2, &TestClass::constVoidVoidMethod);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoCallbacksWithWithDifferentClassPtrAreNotEqual)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    MethodCallback<void> sut2(&m_testClass2, &TestClass::voidVoidMethod);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoConstCallbacksWithWithDifferentMethodPtrAreNotEqual)
{
    ConstMethodCallback<void> sut(&m_testClass, &TestClass::constVoidVoidMethod);
    ConstMethodCallback<void> sut2(&m_testClass, &TestClass::constVoidVoidMethod2);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoCallbacksWithWithDifferentMethodPtrAreNotEqual)
{
    MethodCallback<void> sut(&m_testClass, &TestClass::voidVoidMethod);
    MethodCallback<void> sut2(&m_testClass2, &TestClass::voidVoidMethod2);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, InvalidConstCallbackCalledWillReturnError)
{
    ConstMethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::constVoidVoidMethod);

    auto result = sut();
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error(), MethodCallbackError::UNABLE_TO_CALL_METHOD_ON_NULLPTR_CLASS_PTR);
}

TEST_F(MethodCallback_test, InvalidCallbackCalledWillReturnError)
{
    MethodCallback<void> sut(static_cast<TestClass*>(nullptr), &TestClass::voidVoidMethod);

    auto result = sut();
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error(), MethodCallbackError::UNABLE_TO_CALL_METHOD_ON_NULLPTR_CLASS_PTR);
}

TEST_F(MethodCallback_test, ValidConstCallbackReturnsValue)
{
    ConstMethodCallback<int, int, int> sut(&m_testClass, &TestClass::myConstMethod);

    auto result = sut(4, 5);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 4 * 5);
}

TEST_F(MethodCallback_test, ValidCallbackReturnsValue)
{
    MethodCallback<int, int, int> sut(&m_testClass, &TestClass::myMethod);

    auto result = sut(6, 7);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 6 + 7);
}

