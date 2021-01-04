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

#include "iceoryx_utils/cxx/function.hpp"
#include "test.hpp"

#include <iostream>

using namespace ::testing;
using namespace iox::cxx;

namespace
{
using std::cout;
using std::endl;

constexpr uint64_t Bytes = 128U;

using signature = int32_t(int32_t);
template <typename T>
using fixed_size_function = iox::cxx::function<T, Bytes>;
using test_function = fixed_size_function<signature>;


// helper template to count construction and copy statistics,
// for our purpose (test_function) we do not need to distinguish
// between copy (move) ctor and copy(move) assignment
template <typename T>
class Counter
{
  public:
    static uint64_t numCreated;
    static uint64_t numCopied;
    static uint64_t numMoved;
    static uint64_t numDestroyed;

    Counter()
    {
        ++numCreated;
    }

    Counter(const Counter&)
    {
        ++numCreated;
        ++numCopied;
    }

    Counter(Counter&&)
    {
        ++numMoved;
    }

    ~Counter()
    {
        ++numDestroyed;
    }

    Counter& operator=(const Counter&)
    {
        ++numCopied;
    }

    Counter& operator=(Counter&&)
    {
        ++numMoved;
    }

    static void resetCounts()
    {
        numCreated = 0U;
        numCopied = 0U;
        numMoved = 0U;
        numDestroyed = 0U;
    }
};

template <typename T>
uint64_t Counter<T>::numCreated = 0U;

template <typename T>
uint64_t Counter<T>::numCopied = 0U;

template <typename T>
uint64_t Counter<T>::numMoved = 0U;

template <typename T>
uint64_t Counter<T>::numDestroyed = 0U;


class Functor : public Counter<Functor>
{
  public:
    Functor(int32_t state)
        : Counter<Functor>()
        , m_state(state)
    {
    }

    int32_t operator()(int32_t n)
    {
        m_state += n;
        return m_state;
    }

    // integer arg to satisfy signature requirement of our test_function
    int32_t getState(int32_t n = 0) const
    {
        return m_state + n;
    }

    int32_t m_state{0};
};

int32_t freeFunction(int32_t n)
{
    return n + 1;
};


class function_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    static int32_t staticFunction(int32_t n)
    {
        return n + 1;
    }
};

TEST_F(function_test, DefaultConstructionCreatesNoCallable)
{
    test_function sut;

    EXPECT_FALSE(sut.operator bool());
}

TEST_F(function_test, ConstructionFromFunctorIsCallable)
{
    Functor f(73);
    Functor::resetCounts();
    test_function sut(f);

    EXPECT_EQ(Functor::numCreated, 1U);
    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), f(1));
}

TEST_F(function_test, ConstructionFromLambdaIsCallable)
{
    int32_t capture = 37;
    auto lambda = [state = capture](int32_t n) { return state + n; };
    test_function sut(lambda);

    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), lambda(1));
}

TEST_F(function_test, ConstructionFromFreeFunctionIsCallable)
{
    test_function sut(freeFunction);

    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), freeFunction(1));
}

TEST_F(function_test, ConstructionFromStaticFunctionIsCallable)
{
    // is essentially also a free function but we test the case to be sure
    test_function sut(staticFunction);

    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), staticFunction(1));
}

TEST_F(function_test, ConstructionFromMemberFunctionIsCallable)
{
    Functor f(37);
    test_function sut(f, &Functor::operator());

    ASSERT_TRUE(sut.operator bool());
    auto result = f(1);
    EXPECT_EQ(sut(1), result + 1);
}

TEST_F(function_test, ConstructionFromConstMemberFunctionIsCallable)
{
    Functor f(37);
    test_function sut(f, &Functor::getState);

    ASSERT_TRUE(sut.operator bool());
    auto state = f.getState(1);
    EXPECT_EQ(sut(1), state);
    EXPECT_EQ(f.getState(1), state); // state is unchanged by the previous call
}

TEST_F(function_test, ConstructionFromAnotherFunctionIsCallable)
{
    constexpr int32_t INITIAL = 37;
    int32_t capture = INITIAL;
    auto lambda = [&](int32_t n) { return ++capture + n; };
    function<signature, Bytes / 2> f(lambda); // the other function type must be small enough to fit
    test_function sut(f);

    ASSERT_TRUE(sut.operator bool());
    auto result = f(1);
    EXPECT_EQ(sut(1), result + 1);
    EXPECT_EQ(capture, INITIAL + 2);
}

TEST_F(function_test, FunctionStateIsIndependentOfSource)
{
    constexpr uint32_t INITIAL_STATE = 73U;
    static_storage<1024U> storage;
    auto p = storage.allocate<Functor>();
    p = new (p) Functor(INITIAL_STATE);
    auto& functor = *p;

    // test whether the function really owns the functor
    // (no dependency or side effects)
    test_function sut(functor);

    ASSERT_TRUE(sut.operator bool());

    // both increment their state independently
    EXPECT_EQ(sut(1U), functor(1U));

    // clear original (set to 0)
    p->~Functor();
    storage.clear();

    EXPECT_EQ(sut(1U), INITIAL_STATE + 2U);
}

// The implementation uses type erasure and we need to verify that the corresponding
// constructors and operators of the underlying object (functor) are called.

TEST_F(function_test, DestructorCallsDestructorOfStoredFunctor)
{
    Functor f(73);
    Functor::resetCounts();

    {
        test_function sut(f);
    }

    EXPECT_EQ(Functor::numDestroyed, 1U);
}

TEST_F(function_test, CopyCtorCopiesStoredFunctor)
{
    Functor functor(73);
    test_function f(functor);
    Functor::resetCounts();

    test_function sut(f);

    EXPECT_EQ(Functor::numCopied, 1U);
    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), f(1));
}

TEST_F(function_test, MoveCtorMovesStoredFunctor)
{
    Functor functor(73);
    test_function f(functor);
    Functor::resetCounts();

    test_function sut(std::move(f));

    EXPECT_EQ(Functor::numMoved, 1U);
    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), functor(1));
    EXPECT_FALSE(f.operator bool());
}

TEST_F(function_test, CopyAssignmentCopiesStoredFunctor)
{
    test_function f(Functor(73));
    test_function sut(Functor(42));

    Functor::resetCounts();
    sut = f;

    EXPECT_EQ(Functor::numDestroyed, 1U);
    EXPECT_EQ(Functor::numCopied, 1U);
    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), f(1));
}

TEST_F(function_test, MoveAssignmentMovesStoredFunctor)
{
    Functor functor(73);
    test_function f(functor);
    test_function sut(Functor(42));

    Functor::resetCounts();
    sut = std::move(f);

    // destroy previous Functor in sut and Functor in f after move
    // (f is not callable but can be reassigned)
    EXPECT_EQ(Functor::numDestroyed, 2U);
    EXPECT_EQ(Functor::numMoved, 1U);
    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), functor(1));
    EXPECT_FALSE(f.operator bool());
}


TEST_F(function_test, CopyCtorCopiesStoredFreeFunction)
{
    test_function f(freeFunction);
    test_function sut(f);

    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), f(1));
}

TEST_F(function_test, MoveCtorMovesStoredFreeFunction)
{
    test_function f(freeFunction);
    test_function sut(std::move(f));

    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), freeFunction(1));
    EXPECT_FALSE(f.operator bool());
}

TEST_F(function_test, CopyAssignmentCopiesStoredFreeFunction)
{
    test_function f(freeFunction);
    test_function sut(Functor(73));

    Functor::resetCounts();
    sut = f;

    EXPECT_EQ(Functor::numDestroyed, 1U);
    EXPECT_EQ(Functor::numCopied, 0U);
    EXPECT_EQ(Functor::numMoved, 0U);
    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), f(1));
}

TEST_F(function_test, MoveAssignmentMovesStoredFreeFunction)
{
    test_function f(freeFunction);
    test_function sut(Functor(73));

    Functor::resetCounts();
    sut = std::move(f);

    EXPECT_EQ(Functor::numDestroyed, 1U);
    EXPECT_EQ(Functor::numCopied, 0U);
    EXPECT_EQ(Functor::numMoved, 0U);
    ASSERT_TRUE(sut.operator bool());
    EXPECT_EQ(sut(1), freeFunction(1));
    EXPECT_FALSE(f.operator bool());
}

TEST_F(function_test, CopiedNonCallableFunctionIsNotCallable)
{
    test_function f;
    Functor::resetCounts();

    test_function sut(f);

    EXPECT_EQ(Functor::numCopied, 0U);
    EXPECT_EQ(Functor::numMoved, 0U);
    EXPECT_FALSE(sut.operator bool());
}

TEST_F(function_test, MovedNonCallableFunctionIsNotCallable)
{
    test_function f;
    Functor::resetCounts();

    test_function sut(std::move(f));

    EXPECT_EQ(Functor::numCopied, 0U);
    EXPECT_EQ(Functor::numMoved, 0U);
    EXPECT_FALSE(sut.operator bool());
    EXPECT_FALSE(f.operator bool());
}

TEST_F(function_test, CopyAssignedNonCallableFunctionIsNotCallable)
{
    test_function f;
    test_function sut(Functor(73));

    Functor::resetCounts();
    sut = f;

    EXPECT_EQ(Functor::numDestroyed, 1U);
    EXPECT_EQ(Functor::numCopied, 0U);
    EXPECT_EQ(Functor::numMoved, 0U);
    EXPECT_FALSE(sut.operator bool());
    EXPECT_FALSE(f.operator bool());
}

TEST_F(function_test, MoveAssignedNonCallableFunctionIsNotCallable)
{
    test_function f;
    test_function sut(Functor(73));

    Functor::resetCounts();
    sut = std::move(f);

    EXPECT_EQ(Functor::numDestroyed, 1U);
    EXPECT_EQ(Functor::numCopied, 0U);
    EXPECT_EQ(Functor::numMoved, 0U);
    EXPECT_FALSE(sut.operator bool());
    EXPECT_FALSE(f.operator bool());
}

TEST_F(function_test, MemberSwapWorks)
{
    Functor f1(73);
    Functor f2(37);
    test_function sut1(f1);
    test_function sut2(f2);

    sut1.swap(sut2);

    ASSERT_TRUE(sut1.operator bool());
    EXPECT_EQ(sut1(1), f2(1));
    ASSERT_TRUE(sut2.operator bool());
    EXPECT_EQ(sut2(1), f1(1));
}

TEST_F(function_test, StaticSwapWorks)
{
    Functor f1(73);
    Functor f2(37);
    test_function sut1(f1);
    test_function sut2(f2);

    test_function::swap(sut1, sut2);

    ASSERT_TRUE(sut1.operator bool());
    EXPECT_EQ(sut1(1), f2(1));
    ASSERT_TRUE(sut2.operator bool());
    EXPECT_EQ(sut2(1), f1(1));
}


} // namespace