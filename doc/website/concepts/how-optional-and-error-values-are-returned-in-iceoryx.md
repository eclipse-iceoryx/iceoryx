# How optional and error values are returned in iceoryx

Many parts of the iceoryx C++ API follow a functional programming approach and allow the user to specify functions
which handle the possible cases, e.g. what should happen when data is received.

This is very flexible but requires using the monadic types `iox::expected` and `iox::optional`, which we
introduce in the following sections.

## Optional

The type `iox::optional<T>` is used to indicate that there may or may not be a value of a specific type `T`
available. This is essentially the 'maybe [monad](https://en.wikipedia.org/wiki/Monad_%28functional_programming%29)' in
functional programming.

The `iox::optional` behaves like the [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional),
except that it terminates the application where `std::optional` would throw an exception or exhibit undefined behavior.

Assuming we have some optional (usually the result of some computation)

```cpp
optional<int> result = someComputation();
```

we can check for its value using

```cpp
if(result.has_value())
{
    auto value = result.value();
    // do something with the value
}
else
{
    // handle the case that there is no value
}
```

A shorthand to get the value is

```cpp
auto value = *result;
```

!!! attention
    Accessing the value if there is no value terminates the application, so it must be checked beforehand.

We can achieve the same with the functional approach by providing a function for both cases.

```cpp
result.and_then([](int& value) { /*do something with the value*/ })
    .or_else([]() { /*handle the case that there is no value*/ });
```

Notice that we get the value by reference, so if a copy is desired it has to be created explicitly in the
[lambda expression](https://en.wikipedia.org/wiki/Anonymous_function#C++_%28since_C++11%29) or function we pass.

The optional can be initialized from a value directly

```cpp
optional<int> result = 73;
result = 37;
```

If the optional is default initialized, it is automatically set to its null value of type `iox::nullopt_t`.
This can be also done directly by using the constant `iox::nullopt`

```cpp
result = iox::nullopt;
```

For a complete list of available functions see
[`optional.hpp`](../../../iceoryx_hoofs/vocabulary/include/iox/optional.hpp).


## Expected

`iox::expected<T, E>` generalizes `iox::optional` by admitting a value of another type `E` instead of
no value at all, i.e. it contains either a value of type `T` or `E`. In this way, `expected` is a special case of
the 'either monad'. It is usually used to pass a value of type `T` or an error that may have occurred, i.e. `E` is the
error type.

For more information on how it is used for error handling see
[error-handling.md](../../design/error-handling.md).

Assume we have `E` as an error type, then we can create a value

```cpp
iox::expected<int, E> result(iox::ok(73));
```

and use the value or handle a potential error

```cpp
if (result.has_value())
{
    auto value = result.value();
    // do something with the value
}
else
{
    auto error = result.error();
    // handle the error
}
```

If we need an error value, we set

```cpp
result = iox::err(errorCode);
```

which assumes that `E` can be constructed from an `errorCode`.

We again can employ a functional approach like this:

```cpp
auto handleValue = [](int& value) { /*do something with the value*/ };
auto handleError = [](E& value) { /*handle the error*/ };
result.and_then(handleValue).or_else(handleError);
```

There are more convenience functions such as `value_or` which provides the value or an alternative specified by the
user. These can be found in
[`expected.hpp`](../../../iceoryx_hoofs/vocabulary/include/iox/expected.hpp).

Note that when we move an `expected`, the origin contains a moved `T` or `E`, depending on the content before the move.
This reflects the behavior of moving the content out of the `iox::expected` as in
`auto foo = std::move(bar.value());` with `bar` being an `iox::expected`.
Like all objects, `T` and `E` must therefore be in a well defined state after the move.
