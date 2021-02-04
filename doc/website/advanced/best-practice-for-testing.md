# Best practice for testing

This is a guide on how to write tests for iceoryx. It intends to cover the most common cases which will probably apply to 99% of the tests.
This shall not encourage to be more royalist than the king and common sense shall be applied when the guidelines don't make sense.

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
- etc.

Following [Hyrum's Law](https://www.hyrumslaw.com/) loosely, given enough users, one will find ways to use the software in a way it was never imagined. Therefore never underestimate the creativity of brilliancy/stupidity.

# Bad Example
- bad example with `Number` class and add/subtract/multiply/division in one test case with bad name
- is it really a good idea to show how to not do it?

# Good Examples
- AAA pattern
- ZOMBIES principle
- meaningful names
- create objects on the stack or instantiate large objects with smart pointer. Avoid manual memory management with new/delete.
- use `sut`
- Example with assert before accessing a potential nullptr/nullopt;
- typed tests
- parameterized tests
    - https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest
    - use `std::tie`
