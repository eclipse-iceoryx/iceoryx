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
//
// SPDX-License-Identifier: Apache-2.0

#include "test.hpp"

#include "iceoryx_hoofs/cxx/method_callback.hpp"

namespace
{
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
            return a + b + m_id;
        }

        int myConstMethod(int a, int b) const
        {
            return a * b + m_id;
        }

        int myMethod2(int a, int b)
        {
            return a + b + 2 + m_id;
        }

        int myConstMethod2(int a, int b) const
        {
            return a * b + 2 + m_id;
        }

        int m_id = 0;
    };

    TestClass m_testClass;
    TestClass m_testClass2;
};

TEST_F(MethodCallback_test, DefaultMethodCallbackCtorProvidesInvalidCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f034cce-42c0-462a-abed-44ed68d8e64b");
    MethodCallback<void> sut;
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, DefaultConstMethodCallbackCtorProvidesInvalidCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb66c21f-d17e-47b7-8b0f-ab833fb96788");
    ConstMethodCallback<void> sut;
    EXPECT_FALSE(static_cast<bool>(sut));
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, NonDefaultMethodCallbackCtorCreatesValidCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd046467-1171-4b91-b002-6fe8a64ce210");
    MethodCallback<void> sut(m_testClass, &TestClass::voidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, NonDefaultConstMethodCallbackCtorCreatesValidCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "47af06d4-2c80-4fc0-8b18-ae1a4d7f4c97");
    ConstMethodCallback<void> sut(m_testClass, &TestClass::constVoidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, GetObjectPointerReturnsValidConstMethodCallbackClass)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb6ea3d7-22ae-4dd1-8d5d-f3861d70f4f0");
    ConstMethodCallback<void> sut(m_testClass, &TestClass::constVoidVoidMethod);
    EXPECT_EQ(sut.getObjectPointer<TestClass>(), &m_testClass);
}

TEST_F(MethodCallback_test, GetObjectPointerReturnsValidMethodCallbackClass)
{
    ::testing::Test::RecordProperty("TEST_ID", "f565f2cd-8987-4e12-9e3f-88aafbdd8b91");
    MethodCallback<void> sut(m_testClass, &TestClass::voidVoidMethod);
    EXPECT_EQ(sut.getObjectPointer<TestClass>(), &m_testClass);
}

TEST_F(MethodCallback_test, GetMethodPointerReturnsValidConstMethodCallbackClass)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5045551-572d-44c0-950a-a4e42c2ec1fd");
    ConstMethodCallback<void> sut(m_testClass, &TestClass::constVoidVoidMethod);
    EXPECT_EQ(sut.getMethodPointer<TestClass>(), &TestClass::constVoidVoidMethod);
}

TEST_F(MethodCallback_test, GetMethodPointerReturnsValidMethodCallbackClass)
{
    ::testing::Test::RecordProperty("TEST_ID", "3fec6b4f-09a3-4e7b-953f-4924146b3f98");
    MethodCallback<void> sut(m_testClass, &TestClass::voidVoidMethod);
    EXPECT_EQ(sut.getMethodPointer<TestClass>(), &TestClass::voidVoidMethod);
}

TEST_F(MethodCallback_test, AssignCallbackClassPtrLeadsToValidMethodCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "602dacdc-d837-4bc9-8c1d-0c6239bacf64");
    MethodCallback<void> sut;
    sut.setCallback(m_testClass, &TestClass::voidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, AssignCallbackClassPtrLeadsToValidConstMethodCallback)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb14c0b1-cdc1-4ca4-a0b2-f5202e136baa");
    ConstMethodCallback<void> sut;
    sut.setCallback(m_testClass, &TestClass::constVoidVoidMethod);
    EXPECT_TRUE(static_cast<bool>(sut));
    EXPECT_TRUE(sut.isValid());
}

TEST_F(MethodCallback_test, TwoConstCallbacksWithSameClassAndMethodAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "999337af-75a5-4a24-9b88-cb9adc23add1");
    ConstMethodCallback<void> sut(m_testClass, &TestClass::constVoidVoidMethod);
    ConstMethodCallback<void> sut2(m_testClass, &TestClass::constVoidVoidMethod);

    EXPECT_TRUE(sut == sut2);
    EXPECT_FALSE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoCallbacksWithSameClassAndMethodAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "cee095c0-d17a-494e-81e0-868d9b81b565");
    MethodCallback<void> sut(m_testClass, &TestClass::voidVoidMethod);
    MethodCallback<void> sut2(m_testClass, &TestClass::voidVoidMethod);

    EXPECT_TRUE(sut == sut2);
    EXPECT_FALSE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoConstCallbacksWithDifferentClassPtrAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "6114cc26-6537-4242-b9e3-fe18e009c03e");
    ConstMethodCallback<void> sut(m_testClass, &TestClass::constVoidVoidMethod);
    ConstMethodCallback<void> sut2(m_testClass2, &TestClass::constVoidVoidMethod);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoCallbacksWithDifferentClassPtrAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "11bc933b-8800-4a92-a2c3-da599ea9743a");
    MethodCallback<void> sut(m_testClass, &TestClass::voidVoidMethod);
    MethodCallback<void> sut2(m_testClass2, &TestClass::voidVoidMethod);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoConstCallbacksWithDifferentMethodPtrAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0f6cbe3-d18b-4680-b85b-970eb838055e");
    ConstMethodCallback<void> sut(m_testClass, &TestClass::constVoidVoidMethod);
    ConstMethodCallback<void> sut2(m_testClass, &TestClass::constVoidVoidMethod2);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, TwoCallbacksWithDifferentMethodPtrAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "21b418ae-830b-46b6-b0b6-bfbca1b43e2c");
    MethodCallback<void> sut(m_testClass, &TestClass::voidVoidMethod);
    MethodCallback<void> sut2(m_testClass2, &TestClass::voidVoidMethod2);

    EXPECT_FALSE(sut == sut2);
    EXPECT_TRUE(sut != sut2);
}

TEST_F(MethodCallback_test, InvalidConstCallbackCalledWillReturnError)
{
    ::testing::Test::RecordProperty("TEST_ID", "8df63353-1ff3-4f0f-99b1-da4bc9974f63");
    ConstMethodCallback<void> sut;

    auto result = sut();
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error(), MethodCallbackError::UNINITIALIZED_CALLBACK);
}

TEST_F(MethodCallback_test, InvalidCallbackCalledWillReturnError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ed7c558-3620-4627-8b71-cd625b995b6d");
    MethodCallback<void> sut;

    auto result = sut();
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error(), MethodCallbackError::UNINITIALIZED_CALLBACK);
}

TEST_F(MethodCallback_test, ValidConstCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca9ccdfa-1163-4138-96a0-ae9a9345c46c");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);

    auto result = sut(4, 5);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 4 * 5);
}

TEST_F(MethodCallback_test, ValidCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae144df4-c946-496d-bd2d-2119a6977cc7");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);

    auto result = sut(6, 7);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 6 + 7);
}

TEST_F(MethodCallback_test, SetNewCallbackMethodOnConstMethodCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ef8f2b0-ad79-428e-b6ff-c5c6e466ae1e");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);
    sut.setCallback(m_testClass, &TestClass::myConstMethod2);

    auto result = sut(4, 5);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 4 * 5 + 2);
}

TEST_F(MethodCallback_test, SetNewCallbackMethodOnMethodCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f5560c1-5d5e-4d05-8bce-faba7c068eed");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);
    sut.setCallback(m_testClass, &TestClass::myMethod2);

    auto result = sut(6, 7);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 6 + 7 + 2);
}

TEST_F(MethodCallback_test, SetNewCallbackOriginOnConstMethodCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0d52bc0-b6da-42de-819d-f1271699e9be");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);
    sut.setCallback(m_testClass2, &TestClass::myConstMethod);
    m_testClass2.m_id = 567;

    auto result = sut(4, 5);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 4 * 5 + 567);
}

TEST_F(MethodCallback_test, SetNewCallbackOriginOnMethodCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e3b09418-bf13-4cf3-bff2-f1a16cb8d821");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);
    sut.setCallback(m_testClass2, &TestClass::myMethod);
    m_testClass2.m_id = 5671;

    auto result = sut(6, 7);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 6 + 7 + 5671);
}

TEST_F(MethodCallback_test, SetNewCallbackFullOnConstMethodCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3a4fa95-8fab-4e97-a4a0-0a87413545de");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);
    sut.setCallback(m_testClass2, &TestClass::myConstMethod2);
    m_testClass2.m_id = 1567;

    auto result = sut(4, 5);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 4 * 5 + 1567 + 2);
}

TEST_F(MethodCallback_test, SetNewCallbackFullOnMethodCallbackReturnsValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "5949926a-3e8c-468d-bae4-3c16318da4a0");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);
    sut.setCallback(m_testClass2, &TestClass::myMethod2);
    m_testClass2.m_id = 56711;

    auto result = sut(6, 7);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 6 + 7 + 56711 + 2);
}

TEST_F(MethodCallback_test, MoveCTorInvalidatesOriginForConstMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b9b87c4-0d5f-4c18-a571-4fc85bea8fab");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);
    ConstMethodCallback<int, int, int> sut2(std::move(sut));

    EXPECT_TRUE(sut2.isValid());
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, MoveCTorInvalidatesOriginForMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b0c6b27-a9e2-4cb9-b95d-5ce81b7710bf");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);
    MethodCallback<int, int, int> sut2(std::move(sut));

    EXPECT_TRUE(sut2.isValid());
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, MoveAssignmentInvalidatesOriginForConstMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "eac47f6d-7ae2-470b-94e0-780d5029d3f3");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);
    ConstMethodCallback<int, int, int> sut2;
    sut2 = std::move(sut);

    EXPECT_TRUE(sut2.isValid());
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, MoveAssignmentInvalidatesOriginForMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "54f3adab-7a8a-4f69-ac55-0c0ab1cd4151");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);
    MethodCallback<int, int, int> sut2;
    sut2 = std::move(sut);

    EXPECT_TRUE(sut2.isValid());
    EXPECT_FALSE(sut.isValid());
}

TEST_F(MethodCallback_test, MoveCTorDestinationCanCallCallbackForConstMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "e3c90d44-f75b-457d-86c3-80b76597db9c");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);
    ConstMethodCallback<int, int, int> sut2(std::move(sut));

    auto result = sut2(8, 9);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 72);
}

TEST_F(MethodCallback_test, MoveAssignemtnDestinationCanCallCallbackForConstMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9691902-7473-441f-81a0-5066ac76436e");
    ConstMethodCallback<int, int, int> sut(m_testClass, &TestClass::myConstMethod);
    ConstMethodCallback<int, int, int> sut2;
    sut2 = (std::move(sut));

    auto result = sut2(10, 11);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 110);
}

TEST_F(MethodCallback_test, MoveCTorDestinationCanCallCallbackForMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ce34c39-8f22-49d2-97d3-1ff1a9d3a9b9");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);
    MethodCallback<int, int, int> sut2(std::move(sut));

    auto result = sut2(12, 14);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 26);
}

TEST_F(MethodCallback_test, MoveAssignemtnDestinationCanCallCallbackForMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b2ae355-f792-4b96-944e-88942a889f8e");
    MethodCallback<int, int, int> sut(m_testClass, &TestClass::myMethod);
    MethodCallback<int, int, int> sut2;
    sut2 = (std::move(sut));

    auto result = sut2(11, 11);
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result, 22);
}
} // namespace
