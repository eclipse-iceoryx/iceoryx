# Best practice for testing

This is a guide on how to write tests for iceoryx. It intends to cover the most common cases which will probably apply to 99% of the tests.
This shall not encourage to be more royalist than the king and common sense shall be applied when the guidelines don't make sense.

The guide also expects some knowledge on `gtest`. At least the [Googletest Primer](https://github.com/google/googletest/blob/master/docs/primer.md) document should be read before continuing with this guide.

Don't write test just for the sake of having a high coverage number. First and foremost, tests must be meaningful and **verify the code** to prevent bugs and regressions.
New code shall be created with testability in mind. Legacy code shall be refactored if it is not testable.

In general, the **Arrange Act Assert** pattern shall be used. This makes it trivial to isolate a test failure, since only one aspect of the code is tested at a time.
These two [blog](https://defragdev.com/blog/2014/08/07/the-fundamentals-of-unit-testing-arrange-act-assert.html)
[posts](https://medium.com/@pjbgf/title-testing-code-ocd-and-the-aaa-pattern-df453975ab80) explain the **AAA** pattern in more detail.

While the AAA pattern provides a sensible structure, [ZOMBIES](http://blog.wingman-sw.com/tdd-guided-by-zombies) help you to find sensible test cases.
```
Z = Zero
O = One
M = Many (or More complex)
B = Boundary Behaviors
I = Interface Definition
E = Exercise Exceptional Behavior
S = Simple Scenarios, Simple Solutions
```

This can be separated into **ZOM** and **BIE** with **S** bridging them together. **ZOM** are often simple test, like _a vector with zero items is empty_ or _a vector with one item is not empty_ or _a vector with N items has a size of N_.
The **BIE** part takes care of edge cases like _adding one to Number::max saturates_ or _division by zero returns error_. The latter overlaps with a case from the **ZOM** part, which illustrates that these are not always clearly separated.

**Exercise Exceptional Behavior** means to not only test the happy path, but also the negative one. Basically to test with silly inputs and check for a graceful behavior like the previous example with division by 0. The linked blog post explains [negative testing](https://www.softwaretestinghelp.com/what-is-negative-testing/) in a more thorough way.

The catchwords can be used to draw simple scenarios which nicely fits to the AAA pattern. A non-exhaustive list of these scenarios are
- overflow
- underflow
- wrap around
- empty
- full
- out of bounds
- timeouts
- copy & move
- etc.

Following [Hyrum's Law](https://www.hyrumslaw.com/) loosely, given enough users, one will find ways to use the software in a way it was never imagined. Therefore never underestimate the creativity of brilliancy/stupidity.

# Practical Example

Let's test the following class

```c++
class SingleDigitNumber
{
  public:
    SingleDigitNumber() noexcept = default;

    SingleDigitNumber(uint64_t number)
        : m_number(number)
    {
    }

    operator uint64_t()
    {
        return m_number;
    }

    SingleDigitNumber operator+(const SingleDigitNumber rhs)
    {
        return m_number + rhs.m_number;
    }

    SingleDigitNumber operator-(const SingleDigitNumber rhs)
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

## First Attempt

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

The test above has several major and minor flaws. The first thing that leaps to the eye, it doesn't follow the **AAA** pattern. When the test fails, we don't know why and have to look in the code to figure out what went wrong. But even if only one aspect of the class would have been tested, there would still be essentially just a `[  FAILED  ] SingleDigitNumber_test.TestClass` output and we would need to look at the code to know what exactly is broken. This test case checks too many aspects of the code and doesn't have a sensible name.

Furthermore, the default constructor is called in the test, but it is never checked to do the right thing. If there were a check, it would have revealed that `m_number` is not correctly initialized.

Then, `EXPECT_TRUE` is used to compare the values. While this works, when the test fails we just get the information of the failure, but not which values were compared.
To get this information `EXPECT_EQ` or `EXPECT_THAT` should be used. Furthermore, it's important to easily distinguish between the actual and the expected value, therefore the ordering of the values should be the same as if `EXPECT_THAT` would be used. The first value should be the actual value and the second one the expected.

While the coverage might be 100%, there are no tests for:
- invalid parameters, e.g. passing `10` to the constructor
- overflow with `operator+`, e.g. adding `7` and `8`
- underflow with `operator-`, e.g. subtracting `8` from `7`

Here, ZOMBIES comes into play and gives us some guide to identify this cases.

Some of this test might made it necessary to define the behavior of the class in the first place, e.g. defining that an `operator+` will saturate to the max value instead of overflowing.

## How To Do It Better

At first, the test is split into multiple test cases according the AAA pattern

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

TODO

- ZOMBIES principle
- meaningful names
- create objects on the stack or instantiate large objects with smart pointer. Avoid manual memory management with new/delete.
- use `sut`
- Example with assert before accessing a potential nullptr/nullopt;

# Slightly More Advanced Techniques

TODO

- typed tests
- parameterized tests
    - https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest
    - use `std::tie`
- mocks
