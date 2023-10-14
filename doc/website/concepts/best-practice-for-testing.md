# Best practice for testing

This is a guide on how to write tests for iceoryx. It intends to cover the most common cases which will probably apply to 99% of tests.
This shall not encourage to be more royalist than the king and common sense shall be applied when the guidelines don't make sense.
But if half of the tests don't follow the guidelines, it's a clear indicator that they need to be refactored.

The guide also expects some knowledge on `gtest`. At least the [Googletest Primer](https://github.com/google/googletest/blob/master/docs/primer.md) document should be read before continuing with this guide.

Don't write tests just for the sake of having a high coverage number.
First and foremost, tests must be meaningful and **verify the code** to prevent bugs and regressions.
New code shall be created with testability in mind. Legacy code shall be refactored if it is not testable.

In general, the **Arrange Act Assert** pattern shall be used.
This makes it trivial to isolate a test failure, since only one state transition is tested at a time.
These two [blog](https://defragdev.com/blog/2014/08/07/the-fundamentals-of-unit-testing-arrange-act-assert.html)
[posts](https://medium.com/@pjbgf/title-testing-code-ocd-and-the-aaa-pattern-df453975ab80) explain the **AAA** pattern in more detail.

While the AAA pattern provides a sensible structure, [ZOMBIES](http://blog.wingman-sw.com/tdd-guided-by-zombies) help you to find sensible test cases.

```txt
Z = Zero
O = One
M = Many (or More complex)
B = Boundary Behaviors
I = Interface Definition
E = Exercise Exceptional Behavior
S = Simple Scenarios, Simple Solutions
```

This can be separated into **ZOM** and **BIE** with **S** bridging them together.
**ZOM** are often simple tests, like _a vector with zero items is empty_ or _a vector with one item is not empty_ or _a vector with N items has a size of N_.
The **BIE** part takes care of edge cases like _adding one to Number::max saturates_ or _division by zero returns error_.
The latter overlaps with a case from the **ZOM** part, which illustrates that these are not always clearly separated.

**Exercise Exceptional Behavior** means to not only test the happy path, but also the negative one.
Basically, you should test silly inputs and check for a graceful behavior like the previous example with division by 0.
The linked blog post explains [negative testing](https://www.softwaretestinghelp.com/what-is-negative-testing) <!--NOLINT link works but linter fails--> in a more thorough way.

The catchwords can be used to draw simple scenarios which nicely fits to the AAA pattern. A non-exhaustive list of these scenarios are

- overflow
- underflow
- wrap around
- empty
- full
- out of bounds
- timeouts
- copy
  - are the objects equal
  - is the copy origin unchanged
  - etc.
- move
  - is the move destination object cleaning up its resources
  - is the move origin object in a defined but unspecified state
  - etc.
- etc.

Following [Hyrum's Law](https://www.hyrumslaw.com) loosely, given enough users, one will find ways to use the software in a way it was never imagined.
Therefore, never underestimate the creativity of brilliancy/stupidity.

In some cases it might be necessary to instantiate an object on the heap. While that's not allowed in production code, it is fine in test code.
To avoid manual memory management with new/delete, smart pointers shall be used if possible.
As a reminder, if a method takes a pointer to an object, this object can be instantiated on the stack and the address of this object can be passed to the method.
A good reason to use the heap are large objects which might cause a stack overflow.
Some operating systems have a rather small stack of only a few kB, so this limit might be closer one might think.

In general, the tests should be written in a fashion to not crash the application in case of a faulty implementation.
It must be assumed that the implementation is broken and only a successful test run can prove otherwise.
The `sut` (system under test) might return a `nullptr` instead of the expected valid pointer, so `nullptr` check has to be done with an `ASSERT_*` to gracefully abort the current test.
Just using an `EXPECT_*` for the check is not sufficient since the potential `nullptr` will be dereferenced later on and will crash the application.
The same applies to other potential dangerous operations, like accessing the value of a `iox::optional` or `iox::expected` or an out of bounds access of a `iox::vector`.

Last but not least, apply the **DRY** principle (don't repeat yourself) and use typed and parameterized tests to check multiple implementations and variations without repeating much code.

## Practical Example

Let's test the following class

```c++
class SingleDigitNumber
{
  public:
    SingleDigitNumber() noexcept = default;

    constexpr SingleDigitNumber(uint64_t number) noexcept
        : m_number(number)
    {
    }

    constexpr operator uint64_t() const noexcept
    {
        return m_number;
    }

    constexpr SingleDigitNumber operator+(const SingleDigitNumber rhs) const noexcept
    {
        return m_number + rhs.m_number;
    }

    constexpr SingleDigitNumber operator-(const SingleDigitNumber rhs) const noexcept
    {
        return m_number - rhs.m_number;
    }

  private:
    uint64_t m_number;
};
```

This test fixture will be used

```c++
class SingleDigitNumber_test : public Test
{
  public:
    void SetUp() override{};
    void TearDown() override{};
};
```

### First Attempt

Well, this is quite a simple class, so the tests must also be simple, right?

```c++
TEST_F(SingleDigitNumber_test, TestClass)
{
    SingleDigitNumber number1;
    SingleDigitNumber number2(3U);
    SingleDigitNumber number3(2U);

    auto number4 = number2 + number3;
    auto number5 = number2 - number3;

    EXPECT_TRUE(number4 == 5U);
    EXPECT_EQ(1U, static_cast<uint64_t>(number5));
}
```

Done and we reached 100% coverage. We can clap ourselves on the shoulders and move on. Well, except that we can't.

The test above has several major and minor flaws. The first thing that leaps to the eye, it doesn't follow the **AAA** pattern. When the test fails, we don't know why and have to look in the code to figure out what went wrong. But even if only one aspect of the class would have been tested, there would still be essentially just a `[  FAILED  ] SingleDigitNumber_test.TestClass` output and we would need to look at the code to know what exactly is broken. This test case checks too many state transitions and doesn't have a sensible name.

Furthermore, the default constructor is called in the test, but it is never checked to do the right thing. If there was a check, it would have revealed that `m_number` is not correctly initialized.

Then, `EXPECT_TRUE` is used to compare the values. While this works, when the test fails we just get the information of the failure, but not which values were compared.
To get this information `EXPECT_EQ` or `EXPECT_THAT` should be used. Furthermore, it's important to easily distinguish between the actual and the expected value, therefore the ordering of the values should be the same as if `EXPECT_THAT` would be used. The first value should be the actual value and the second one the expected.

While the coverage might be 100%, there are no tests for:

- invalid parameters, e.g. passing `10` to the constructor
- overflow with `operator+`, e.g. adding `7` and `8`
- underflow with `operator-`, e.g. subtracting `8` from `7`

Here, ZOMBIES comes into play and gives us some guide to identify this cases.

Some of this test might made it necessary to define the behavior of the class in the first place, e.g. defining that an `operator+` will saturate to the max value instead of overflowing.

### How To Do It Better

At first, the test is split into multiple test cases according to the AAA pattern

```c++
TEST_F(SingleDigitNumber_test, DefaultConstructedObjectIsCorrectlyInitialized)
{
    constexpr uint64_t EXPECTED_VALUE{0U};

    SingleDigitNumber sut;

    EXPECT_EQ(static_cast<uint64_t>(sut), EXPECTED_VALUE);
}
```

The test also has a meaningful name. If this fails in the CI, it is immediately clear what is broken.
Additionally, the tested object is called `sut`. This makes it easy to identify the actual test object.
Lastly, a `constexpr` is used for the expected value. This removes a magic value and also makes the output of a failed test more readable, since it is immediately clear what's the actual tested value and what's the expected value.

Now let's continue with further tests, applying the ZOMBIES principle

```c++
TEST_F(SingleDigitNumber_test, ConstructionWithValidValueCreatesNumberWithSameValue)
{
    constexpr uint64_t NUMBER_VALUE{7U};
    constexpr uint64_t EXPECTED_VALUE{NUMBER_VALUE};

    SingleDigitNumber sut{NUMBER_VALUE};

    EXPECT_EQ(static_cast<uint64_t>(sut), EXPECTED_VALUE);
}

TEST_F(SingleDigitNumber_test, ConstructionWithInvalidValueResultsInSaturation)
{
    constexpr uint64_t NUMBER_VALUE{42U};
    constexpr uint64_t EXPECTED_VALUE{9U};

    SingleDigitNumber sut{NUMBER_VALUE};

    EXPECT_EQ(static_cast<uint64_t>(sut), EXPECTED_VALUE);
}

TEST_F(SingleDigitNumber_test, AddingZeroDoesNotChangeTheNumber)
{
    constexpr SingleDigitNumber NUMBER{3U};
    constexpr SingleDigitNumber ZERO{0U};
    constexpr uint64_t EXPECTED_VALUE{3U};

    auto sut = NUMBER + ZERO;

    EXPECT_EQ(static_cast<uint64_t>(sut), EXPECTED_VALUE);
}

TEST_F(SingleDigitNumber_test, AddingOneIncreasesTheNumberByOne)
{
    constexpr SingleDigitNumber NUMBER{3U};
    constexpr SingleDigitNumber ONE{1U};
    constexpr uint64_t EXPECTED_VALUE{4U};

    auto sut = NUMBER + ONE;

    EXPECT_EQ(static_cast<uint64_t>(sut), EXPECTED_VALUE);
}

// and so on
```

These are some examples showing how to apply the ZOMBIES principle to find good test cases.
They also exert all the good practices mentioned previously, like clear and distinct names or avoiding magic numbers.

## Slightly More Advanced Topics

### Typed Tests

There are situations when test cases only vary in the type applied to the `sut`. In this case typed tests can be used to reduce repetition.
The there is a section for [typed tests](https://github.com/google/googletest/blob/master/docs/advanced.md#typed-tests) in the advanced gtest documentation.

A more thorough [example](https://github.com/google/googletest/blob/release-1.10.0/googletest/samples/sample6_unittest.cc) is in the gtest github repository.

### Parameterized Tests

Similar to typed tests, there are cases where the same test case should run with multiple parameters.
One example is the conversion of `enum` values to strings. While this can be done in a loop, parameterized testing is a better approach.

[This](https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest) is quite a good blog post to get into parameterized tests. Additionally, there is a section in the advanced [documentation](https://github.com/google/googletest/blob/master/docs/advanced.md#value-parameterized-tests) for gtest.

The block post mentions tuples to pass multiple parameters at once. Since tuples can become cumbersome to use, especially if parameters are rearranged, it is recommended to create a `struct` to wrap the parameters instead.

### Mocks

Some classes are hard to test or to reach a full coverage. This might be due to external access or interaction with the operating system.
Mocks can help to have full control over the `sut` and reliably cause error conditions to test the negative code path.
There is an [extensive gmock documentation](https://github.com/google/googletest/tree/release-1.10.0/googlemock/docs) in the gtest github repository.

### Pitfalls

Some tests require creating dummy classes and it might be that the same name is chosen multiple times, e.g. “class DummyData {...};”.
Usually, at some time the compiler would complain about double definitions.
But since the definitions are not in header but source files and therefore confined in a translation unit, this is not the case and the test binary gets created.
There might still be issues though, since the binary contains multiple symbols with the same name.
One of these issues may arise with the usage of sanitizers, e.g. the address or leak sanitizer.
If there are multiple “DummyData” classes with different sizes and they are created on the heap, which is totally fine for tests, the address sanitizer might detect an error since something with a different size than expected will be freed.
To prevent such issues, the tests should be placed within an anonymous namespace which makes all the symbols unique.

```cpp
namespace
{
struct DummyData {
    uint32_t foo{0};
};

class MyTest : public Test
{
    //...
};

TEST_F(MyTest, TestName)
{
    EXPECT_EQ(ANSWER, 42)
}

} // namespace
```

## Conclusion

- apply the AAA pattern to structure the test and check only one state transition per test case (all side effects of that transition must be checked, though)
- don't test previously tested behavior
- use the ZOMBIES principle to find sensible test cases
- use meaningful names for the tests to indicate what the test is supposed to do and the expected outcome, like `...IsSuccessful`, `...Fails`, `...ResultsIn...`, `...LeadsTo...`, etc.
- name the test object `sut` to make clear which object is tested
- don't use magic numbers
- instantiate objects on the stack or use smart pointers for large objects and avoid manual memory management with new/delete
- use `ASSERT_*` before doing a potential dangerous action which might crash the test application, like accessing a `nullptr` or a `iox::optional` with a `iox::nullopt`
- use mocks to reduce the complexity of the test arrangement
- apply the **DRY** principle by using typed and parameterized tests
- wrap the tests in an anonymous namespace
