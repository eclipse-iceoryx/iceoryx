// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/variant.hpp"
#include "test.hpp"

#include <string>
namespace
{
using namespace testing;

class variant_Test : public Test
{
  public:
    void SetUp() override
    {
        internal::CaptureStderr();
        DoubleDelete::dtorCalls = 0;
        DoubleDelete::ctorCalls = 0;
        DTorTest::dtorWasCalled = false;
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    class ComplexClass
    {
      public:
        ComplexClass(int a, float b)
            : a(a)
            , b(b)
        {
        }

        int a;
        float b;
    };

    struct DTorTest
    {
        ~DTorTest()
        {
            dtorWasCalled = true;
        }
        static bool dtorWasCalled;
    };

    class DoubleDelete
    {
      public:
        DoubleDelete()
            : doDtorCall{true}
        {
            ctorCalls++;
        }
        ~DoubleDelete()
        {
            Delete();
        }
        DoubleDelete(const DoubleDelete& rhs)
            : doDtorCall{false}
        {
            *this = rhs;
        }
        DoubleDelete(DoubleDelete&& rhs)
            : doDtorCall{false}
        {
            *this = std::move(rhs);
        }

        DoubleDelete& operator=(const DoubleDelete& rhs)
        {
            if (this != &rhs)
            {
                Delete();
                doDtorCall = rhs.doDtorCall;
            }
            return *this;
        }

        DoubleDelete& operator=(DoubleDelete&& rhs)
        {
            if (this != &rhs)
            {
                Delete();
                doDtorCall = std::move(rhs.doDtorCall);
                rhs.doDtorCall = false;
            }
            return *this;
        }

        void Delete()
        {
            if (doDtorCall)
            {
                dtorCalls++;
            }
        }
        static int dtorCalls;
        static int ctorCalls;

      private:
        bool doDtorCall;
    };

    iox::cxx::variant<int, float, ComplexClass> sut;
};
bool variant_Test::DTorTest::dtorWasCalled = false;
int variant_Test::DoubleDelete::dtorCalls = 0;
int variant_Test::DoubleDelete::ctorCalls = 0;

TEST_F(variant_Test, DefaultCTorCreatesInvalidVariant)
{
    EXPECT_THAT(sut.index(), Eq(iox::cxx::INVALID_VARIANT_INDEX));
}

TEST_F(variant_Test, InitializedVariantReturnsCorrectIndex)
{
    sut.emplace<float>(1231.22F);
    EXPECT_THAT(sut.index(), Eq(1U));
}

TEST_F(variant_Test, CreatingVariantFromPODTypeReturnsProvidedValue)
{
    iox::cxx::variant<ComplexClass, float> sut2{42.42F};

    ASSERT_THAT(sut2.index(), Eq(1U));
    ASSERT_THAT(sut2.get<float>(), Ne(nullptr));
    EXPECT_THAT(*sut2.get<float>(), Eq(42.42F));
}

TEST_F(variant_Test, CreatingVariantFromLValueReturnsProvidedValue)
{
    std::string string("Buhh");
    iox::cxx::variant<std::string, float> sut2{string};
    ASSERT_THAT(sut2.index(), Eq(0U));
    ASSERT_THAT(sut2.get<std::string>(), Ne(nullptr));
    EXPECT_THAT(sut2.get<std::string>()->c_str(), StrEq("Buhh"));
}

TEST_F(variant_Test, CreatingVariantWithSameTypeChoosesFirstFittingType)
{
    iox::cxx::variant<float, float> sut2{73.73F};

    ASSERT_THAT(sut2.index(), Eq(0U));
    ASSERT_THAT(sut2.get<float>(), Ne(nullptr));
    EXPECT_THAT(*sut2.get<float>(), Eq(73.73F));
}

TEST_F(variant_Test, EmplaceValidElementWorks)
{
    ASSERT_THAT(sut.emplace<ComplexClass>(123, 456.789F), Eq(true));
    ASSERT_THAT(sut.get<ComplexClass>(), Ne(nullptr));
    EXPECT_THAT(sut.get<ComplexClass>()->a, Eq(123));
    EXPECT_THAT(sut.get<ComplexClass>()->b, Eq(456.789F));
}

TEST_F(variant_Test, EmplaceSecondValidElementWorks)
{
    sut.emplace<ComplexClass>(123, 456.789f);
    ASSERT_THAT(sut.emplace<ComplexClass>(912, 65.03F), Eq(true));
    ASSERT_THAT(sut.get<ComplexClass>(), Ne(nullptr));
    EXPECT_THAT(sut.get<ComplexClass>()->a, Eq(912));
    EXPECT_THAT(sut.get<ComplexClass>()->b, Eq(65.03F));
}

TEST_F(variant_Test, DISABLED_emplaceInvalidElement)
{
    // this is a compile time check, if you uncomment this the compiler will
    // fail
    //    EXPECT_THAT(sut.emplace< unsigned int >(0), Eq(false));
}

TEST_F(variant_Test, EmplaceWhenAlreadyDifferentTypeAssignedDoesNotWork)
{
    sut.emplace<int>(123);
    EXPECT_THAT(sut.emplace<float>(123.F), Eq(false));
}

TEST_F(variant_Test, GetOnUninitializedVariantFails)
{
    EXPECT_THAT(sut.get<float>(), Eq(nullptr));
}

TEST_F(variant_Test, GetVariantWithCorrectValueWorks)
{
    sut.emplace<float>(123.12F);
    EXPECT_THAT(sut.get<float>(), Ne(nullptr));
}

TEST_F(variant_Test, GetVariantWithIncorrectValueFails)
{
    sut.emplace<float>(123.12F);
    EXPECT_THAT(sut.get<int>(), Eq(nullptr));
}

TEST_F(variant_Test, ConstGetOnUninitializedVariantFails)
{
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->get<float>(), Eq(nullptr));
}

TEST_F(variant_Test, constGetVariantWithCorrectValue)
{
    sut.emplace<float>(123.12F);
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->get<float>(), Ne(nullptr));
}

TEST_F(variant_Test, ConstGetVariantWithIncorrectValueFails)
{
    sut.emplace<float>(123.12F);
    EXPECT_THAT(const_cast<const decltype(sut)*>(&sut)->get<int>(), Eq(nullptr));
}

TEST_F(variant_Test, Get_ifWhenUninitializedReturnsProvidedValue)
{
    float bla;
    EXPECT_THAT(sut.get_if<float>(&bla), Eq(&bla));
}

TEST_F(variant_Test, Get_ifInitializedWithCorrectValueWorks)
{
    sut.emplace<float>(12.1F);
    float bla;
    EXPECT_THAT(sut.get_if<float>(&bla), Ne(&bla));
}

TEST_F(variant_Test, Get_ifInitializedWithIncorrectValueReturnsProvidedValue)
{
    sut.emplace<float>(12.1F);
    int bla;
    EXPECT_THAT(sut.get_if<int>(&bla), Eq(&bla));
}

TEST_F(variant_Test, DTorIsCalled)
{
    DTorTest::dtorWasCalled = false;
    {
        iox::cxx::variant<int, DTorTest> schlomo;
        schlomo.emplace<DTorTest>();
    }
    EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
}

TEST_F(variant_Test, DTorIsCalledAfterEmplace)
{
    {
        iox::cxx::variant<int, float, DTorTest, double> ignatz;
        ignatz.emplace<DTorTest>();
        DTorTest::dtorWasCalled = false;
    }
    EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
}

TEST_F(variant_Test, CopyCTorWithValueLeadsToSameValue)
{
    iox::cxx::variant<int, char> schlomo;
    schlomo.emplace<int>(123);
    iox::cxx::variant<int, char> ignatz(schlomo);
    ASSERT_THAT(ignatz.get<int>(), Ne(nullptr));
    EXPECT_THAT(*ignatz.get<int>(), Eq(123));
}

TEST_F(variant_Test, CopyCTorWithoutValueResultsInInvalidVariant)
{
    iox::cxx::variant<int, char> schlomo;
    iox::cxx::variant<int, char> ignatz(schlomo);
    ASSERT_THAT(ignatz.index(), Eq(iox::cxx::INVALID_VARIANT_INDEX));
}

TEST_F(variant_Test, CopyAssignmentWithValueLeadsToSameValue)
{
    iox::cxx::variant<int, char> ignatz;
    ignatz.emplace<char>('c');
    {
        iox::cxx::variant<int, char> schlomo;
        schlomo.emplace<int>(447);
        ignatz = schlomo;
    }
    ASSERT_THAT(ignatz.get<int>(), Ne(nullptr));
    ASSERT_THAT(*ignatz.get<int>(), Eq(447));
}

TEST_F(variant_Test, CopyAssignmentWithoutValueResultsInInvalidVariant)
{
    iox::cxx::variant<int, char> ignatz;
    ignatz.emplace<char>('c');
    {
        iox::cxx::variant<int, char> schlomo;
        ignatz = schlomo;
    }
    ASSERT_THAT(ignatz.index(), Eq(iox::cxx::INVALID_VARIANT_INDEX));
    ASSERT_THAT(ignatz.get<char>(), Eq(nullptr));
}

TEST_F(variant_Test, MoveCTorWithValueLeadsToSameValue)
{
    iox::cxx::variant<int, char> schlomo;
    schlomo.emplace<int>(123);
    iox::cxx::variant<int, char> ignatz(std::move(schlomo));
    ASSERT_THAT(ignatz.get<int>(), Ne(nullptr));
    EXPECT_THAT(*ignatz.get<int>(), Eq(123));
    EXPECT_THAT(schlomo.index(), Eq(0U));
}

TEST_F(variant_Test, MoveCTorWithoutValueResultsInInvalidVariant)
{
    iox::cxx::variant<int, char> schlomo;
    iox::cxx::variant<int, char> ignatz(std::move(schlomo));
    ASSERT_THAT(ignatz.index(), Eq(iox::cxx::INVALID_VARIANT_INDEX));
}

TEST_F(variant_Test, MoveAssignmentWithValueLeadsToSameValue)
{
    iox::cxx::variant<int, char> ignatz;
    ignatz.emplace<char>('c');
    {
        iox::cxx::variant<int, char> schlomo;
        schlomo.emplace<int>(447);
        ignatz = std::move(schlomo);
    }
    ASSERT_THAT(ignatz.get<int>(), Ne(nullptr));
    ASSERT_THAT(*ignatz.get<int>(), Eq(447));
}

TEST_F(variant_Test, MoveAssignmentWithoutValueResultsInInvalidVariant)
{
    iox::cxx::variant<int, char> ignatz;
    ignatz.emplace<char>('c');
    {
        iox::cxx::variant<int, char> schlomo;
        ignatz = std::move(schlomo);
    }
    ASSERT_THAT(ignatz.get<int>(), Eq(nullptr));
    ASSERT_THAT(ignatz.index(), Eq(iox::cxx::INVALID_VARIANT_INDEX));
}

TEST_F(variant_Test, CreatingSecondObjectViaCopyCTorResultsInTwoDTorCalls)
{
    {
        iox::cxx::variant<int, DTorTest> ignatz;
        ignatz.emplace<DTorTest>();
        DTorTest::dtorWasCalled = false;
        {
            iox::cxx::variant<int, DTorTest> schlomo(ignatz);
            EXPECT_THAT(DTorTest::dtorWasCalled, Eq(false));
        }
        EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
        DTorTest::dtorWasCalled = false;
    }
    EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
}

TEST_F(variant_Test, CreatingSecondObjectViaCopyAssignmentResultsInTwoDTorCalls)
{
    {
        iox::cxx::variant<int, DTorTest> ignatz;
        ignatz.emplace<DTorTest>();
        DTorTest::dtorWasCalled = false;
        {
            iox::cxx::variant<int, DTorTest> schlomo;
            schlomo.emplace<int>(123);
            schlomo = ignatz;
            EXPECT_THAT(DTorTest::dtorWasCalled, Eq(false));
        }
        EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
        DTorTest::dtorWasCalled = false;
    }
    EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
}

TEST_F(variant_Test, CreatingSecondObjectViaMoveCTorResultsInTwoDTorCalls)
{
    {
        iox::cxx::variant<int, DTorTest> ignatz;
        ignatz.emplace<DTorTest>();
        DTorTest::dtorWasCalled = false;
        {
            iox::cxx::variant<int, DTorTest> schlomo(std::move(ignatz));
            EXPECT_THAT(DTorTest::dtorWasCalled, Eq(false));
            EXPECT_THAT(ignatz.index(), Eq(1U));
        }
        EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
        DTorTest::dtorWasCalled = false;
    }
    EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
}

TEST_F(variant_Test, CreatingSecondObjectViaMoveAssignmentResultsInTwoDTorCalls)
{
    {
        iox::cxx::variant<int, DTorTest> ignatz;
        ignatz.emplace<DTorTest>();
        DTorTest::dtorWasCalled = false;
        {
            iox::cxx::variant<int, DTorTest> schlomo;
            schlomo.emplace<int>(123);
            schlomo = std::move(ignatz);
            EXPECT_THAT(ignatz.index(), Eq(1U));
            EXPECT_THAT(DTorTest::dtorWasCalled, Eq(false));
        }
        EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
        DTorTest::dtorWasCalled = false;
    }
    EXPECT_THAT(DTorTest::dtorWasCalled, Eq(true));
}

TEST_F(variant_Test, DirectValueAssignmentResultsInCorrectIndex)
{
    iox::cxx::variant<int, float> schlomo;
    schlomo = 123;
    EXPECT_THAT(schlomo.index(), Eq(0U));
}

TEST_F(variant_Test, DirectValueAssignmentWhenAlreadyAssignedWithDifferentType)
{
    iox::cxx::variant<int, float> schlomo;
    schlomo = 123;
    schlomo = 123.01F;
    EXPECT_THAT(schlomo.index(), Eq(0U));
}

TEST_F(variant_Test, HoldsAlternativeForCorrectType)
{
    iox::cxx::variant<int, float> schlomo;
    schlomo = 123;
    EXPECT_THAT(iox::cxx::holds_alternative<int>(schlomo), Eq(true));
}

TEST_F(variant_Test, HoldsAlternativeForIncorrectType)
{
    iox::cxx::variant<int, float> schlomo;
    schlomo = 123;
    EXPECT_THAT(iox::cxx::holds_alternative<float>(schlomo), Eq(false));
}

TEST_F(variant_Test, SameTypeVariantAndEmplaceWithIndexResultsInCorrectValue)
{
    iox::cxx::variant<int, float, int> schlomo;

    ASSERT_THAT(schlomo.emplace_at_index<2>(123), Eq(true));
    EXPECT_THAT(*schlomo.get_at_index<2>(), Eq(123));
}

TEST_F(variant_Test, SameTypeVariantResultsInCorrectIndex)
{
    iox::cxx::variant<int, float, int> schlomo;

    EXPECT_THAT(schlomo.emplace_at_index<1>(1.23F), Eq(true));
    EXPECT_THAT(schlomo.index(), Eq(1U));
}

TEST_F(variant_Test, SameTypeVariantReturnsNothingForIncorrectIndex)
{
    iox::cxx::variant<int, float, int> schlomo;

    ASSERT_THAT(schlomo.emplace_at_index<2>(123), Eq(true));
    EXPECT_THAT(schlomo.get_at_index<1>(), Eq(nullptr));
}

TEST_F(variant_Test, ConstSameTypeVariantAndEmplaceWithIndexResultsInCorrectValue)
{
    iox::cxx::variant<int, float, int> schlomo;
    const iox::cxx::variant<int, float, int>* ignatz = &schlomo;

    ASSERT_THAT(schlomo.emplace_at_index<2>(4123), Eq(true));
    EXPECT_THAT(*ignatz->get_at_index<2>(), Eq(4123));
}

TEST_F(variant_Test, InPlaceAtIndexCTorResultsInCorrectIndexAndValue)
{
    iox::cxx::variant<int, float, int> schlomo(iox::cxx::in_place_index<0>(), 445);

    ASSERT_THAT(schlomo.index(), Eq(0U));
    EXPECT_THAT(*schlomo.get_at_index<0>(), Eq(445));
}

TEST_F(variant_Test, InPlaceAtTypeCTorResultsInCorrectIndexAndValue)
{
    iox::cxx::variant<int, float, double> schlomo(iox::cxx::in_place_type<double>(), 90.12);

    ASSERT_THAT(schlomo.index(), Eq(2U));
    EXPECT_THAT(*schlomo.get_at_index<2>(), Eq(90.12));
}

TEST_F(variant_Test, ComplexDTorUsingWrongTypeResultsInNoDTorCall)
{
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> schlomo(iox::cxx::in_place_type<int>(), 90);
    }

    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(0));
}

TEST_F(variant_Test, ComplexDTorUsingCorrectTypeWithEmplace)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> schlomo;
        schlomo.emplace<DoubleDelete>();
    }

    EXPECT_THAT(DoubleDelete::ctorCalls, Eq(1));
    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(1));
}

TEST_F(variant_Test, ComplexDTorUsingCorrectTypeWithInPlace)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> schlomo{iox::cxx::in_place_type<DoubleDelete>()};
    }

    EXPECT_THAT(DoubleDelete::ctorCalls, Eq(1));
    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(1));
}

TEST_F(variant_Test, ComplexDTorWithCopyCTor)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> schlomo{iox::cxx::in_place_type<DoubleDelete>()};
        iox::cxx::variant<int, DoubleDelete> sut{schlomo};
    }

    EXPECT_THAT(DoubleDelete::ctorCalls, Eq(1));
    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(2));
}

TEST_F(variant_Test, ComplexDTorWithCopyAssignmentTwoVariantsWithValue)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> schlomo{iox::cxx::in_place_type<DoubleDelete>()};
        iox::cxx::variant<int, DoubleDelete> sut{iox::cxx::in_place_type<DoubleDelete>()};
        sut = schlomo;
    }

    EXPECT_THAT(DoubleDelete::ctorCalls, Eq(2));
    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(3));
}

TEST_F(variant_Test, ComplexDTorWithMove)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> schlomo{iox::cxx::in_place_type<DoubleDelete>()};
        iox::cxx::variant<int, DoubleDelete> sut = std::move(schlomo);
    }

    EXPECT_THAT(DoubleDelete::ctorCalls, Eq(1));
    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(1));
}

TEST_F(variant_Test, ComplexDTorWithMoveAssignment)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> sut;
        iox::cxx::variant<int, DoubleDelete> schlomo{iox::cxx::in_place_type<DoubleDelete>()};
        sut = std::move(schlomo);
    }

    EXPECT_THAT(DoubleDelete::ctorCalls, Eq(1));
    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(1));
}

TEST_F(variant_Test, ComplexDTorWithMoveAssignmentTwoVariantsWithValue)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    {
        iox::cxx::variant<int, DoubleDelete> sut{iox::cxx::in_place_type<DoubleDelete>()};
        iox::cxx::variant<int, DoubleDelete> schlomo{iox::cxx::in_place_type<DoubleDelete>()};
        sut = std::move(schlomo);
    }

    EXPECT_THAT(DoubleDelete::ctorCalls, Eq(2));
    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(2));
}

TEST_F(variant_Test, MoveVariantIntoVariantOfDifferentType)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    iox::cxx::variant<DoubleDelete, ComplexClass> sut1, sut2;
    sut1.emplace<DoubleDelete>();
    sut2.emplace<ComplexClass>(12, 12.12F);

    sut1 = std::move(sut2);

    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(1));
}

TEST_F(variant_Test, CopyVariantIntoVariantOfDifferentType)
{
    DoubleDelete::ctorCalls = 0;
    DoubleDelete::dtorCalls = 0;
    iox::cxx::variant<DoubleDelete, ComplexClass> sut1, sut2;
    sut1.emplace<DoubleDelete>();
    sut2.emplace<ComplexClass>(12, 12.12F);

    sut1 = sut2;

    EXPECT_THAT(DoubleDelete::dtorCalls, Eq(1));
}
} // namespace
