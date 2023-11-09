# Logging and error handling

## Logging

Logging is performed by the iceoryx internal logger.
The logger is used internally to record errors and general system runtime information. It can also be used by the user in application code for the same purpose.

### Logger

The logger is responsible for logging information about the state of the system to a configurable stream (includes files).

The logger is thread-safe and can hence be safely used from multiple threads concurrently. Currently the logger is synchronous.

### Log levels

The following log levels are supported, ordered by the amount of information displayed from highest to lowest.

* ``VERBOSE`` - all available information is displayed
* ``DEBUG`` - information to support debugging on developer side
* ``INFO`` - run state information for the user
* ``WARN`` - indicates a potential problem which requires investigation
* ``ERR`` - an error occurred that may be handled on application side or by RouDi
* ``FATAL`` - an error occurred and RouDi is unable to continue
* ``OFF`` - no logging information

For ``ERR`` and ``FATAL`` see also error levels ``MODERATE``, ``SEVERE`` (logged with ``LogErr``) and ``FATAL`` (logged with ``LogFatal``) in [Error Levels](#Error-Levels). The levels ``ERR`` and ``FATAL`` are only supposed to be used together with the error handler, i.e. need to be accompanied with a corresponding error handler call (currently this cannot be enforced).

## Error handling

Errors are considered to be system states that should not be reached regularly and usually are the result of an external failure, such as when the OS is unable to provide a certain resource (e.g. a semaphore) or an application does not respond. In contrast, regular behavior such as a receiver receiving no data when none was sent is not an error. On the other hand, losing data that was sent would be considered an error.

There are two general approaches to deal with errors:

1. using exceptions
2. return codes combined with control flow statements and a central instance to handle errors that cannot be mitigated otherwise (the error handler).

In iceoryx we use the latter approach.

### Exceptions

The use of exceptions in iceoryx is prohibited, including third party code that may throw them. This is due to the following reasons:

* In many implementations exception handling requires dynamic memory (even when the exceptions themselves are not generated dynamically via e.g. new).
* Exception handling is not deterministic with respect to runtime.
* Exception handling may cause a (slight) runtime overhead even when no exceptions are thrown.
* In general it is not possible to incorporate (complete) information about all exceptions that can be thrown in the signature of functions. This makes it hard for the caller to decide whether some exception needs to be handled.
* Overuse of exceptions often leads to convoluted try-catch blocks which makes the code hard to maintain.

#### Use of `noexcept`

All functions are marked as ``noexcept``. Note that this does not mean that exceptions cannot be thrown inside such a function. Instead when an exception is thrown inside the function (either directly or indirectly by another function) this exception is not propagated further and ``std::terminate`` will be invoked, calling the terminate handler. The goal is that this cannot happen under any circumstances during runtime.

To this end it is necessary to eliminate the use of all potentially throwing (third party) functions throughout the codebase. Since many STL functions may throw, these cannot be used either and their functionality needs to be reimplemented without the use of exceptions. In particular anything allocating dynamic memory may throw a ``std::bad_alloc`` exception when memory is exhausted.

#### Alternatives to exceptions

As an alternative to exceptions we use the error handler and a variation of return codes in the form of ``iox::expected``, described below. ``iox::expected`` can be used to communicate the error to the caller, who has to decide whether to handle the error itself or propagate it further (e.g. as another ``iox::expected``). Error handling itself is performed by the error handler which handles errors occurring in the subcomponents of ``iceoryx::posh``.

### Error handler

The error handler is called internally when an error is detected in the iceoryx middleware daemon (RouDi) or the iceoryx runtime. The error handler should only be called in exceptional situations (invalid access errors, out of resources etc.) and not in circumstances that occur regularly (it is sort of an exception replacement).

If the exceptional situation can be resolved without calling the error handler (e.g. by delegating an appropriate return value to the caller), this should be preferred (since the error handler is a last resort mechanism).

It is not supposed to be called by applications at any time.

### Technical requirements

* The error handler must be reentrant.
* The error handler must be thread-safe.
* The error handler uses the logger but the logger cannot depend on the error handler.
* When a fatal error is detected and termination is required, the reporting thread shuts down the RouDi gracefully.
* A custom error handler function can be installed (e.g. for testing).
* When the reaction on an error is just logging, computation in other threads shall not be influenced.
* If the error does not require termination, the error handler must return eventually.

### Error Levels

The following error levels are supported.

* ``MODERATE``
* ``SEVERE``
* ``FATAL``

#### MODERATE

A recoverable error. Leads to an error log entry (``LogErr``) and continues execution.

**Example:**

1) RouDi receives an unexpected message and discards it. The remaining communication proceeds normally.
2) A port requested by an application cannot be provided due to e.g. resource exhaustion.

#### SEVERE

RouDi may continue but applications may be compromised or the functionality reduced. Leads to an error log entry (``LogErr``) and assert, terminating execution in debug mode. The handler must return to be able to continue execution in release mode.

**Example:**
A message queue is overflowing and messages are lost. RouDi can continue but lost data may affect applications.

#### FATAL

RouDi cannot continue and will shut down. Leads to an error log entry (``LogFatal``), assert and calls ``std::terminate``, terminating execution in debug and release mode.
The handler is not required to return here (since this may not be always possible or reasonable). The reporting code should still try to proceed to a safe state if possible in order to improve testability in case of such errors.

A fatal error in the runtime terminates the application.

**Example:**
RouDi is unable to allocate sufficient shared memory.

### Error codes and additional information

Currently, error codes are used to identify the location of an error. These are provided as an enum. To uniquely identify an error location, these have to be different for each error. While this does not necessarily have to be the case, it is currently advised as there is no additional information (i.e. line, file) yet which allows to distinguish between different errors with the same code.

Each module has it's own range of errors. The ranges are defined `error_handler.hpp` and the error codes in `error_handling.hpp` for each module. More details in the API reference.

### `IOX_EXPECTS` and `IOX_ENSURES`

These assert-like constructs are used to document assumptions in the code which are checked (at least) in debug mode. Currently they are always active, i.e. also checked in release mode. If the condition is violated they print the condition, the location of occurrence in the code and terminate the program execution.

Since they are not necessarily active in release mode, they cannot be used to handle errors. Their purpose is to detect misuse or bugs of the API early in debug mode or to verify a result of an algorithm before returning. In this way, assumptions of the developer are made explicit without causing overhead when not needed. Therefore errors to be caught by IOX_EXPECTS and IOX_ENSURES are considered bugs and need to be fixed or the underlying assumptions and algorithms changed. This is in contrast to errors which are expected to occur during runtime which are handled by the error handler (i.e. a system resource cannot be obtained).

Although IOX_EXPECTS end IOX_ENSURES behave the same, the former is used to signify a precondition (e.g. arguments of a function) is checked, while the latter indicates a postcondition check (e.g. result of a function before returning)

Examples include expecting pointers that are not null (as input, intermediate or final result) or range checks of variables.

### `iox::expected`

``iox::expected<T, E>`` is a template which either holds the result of the computation of type ``T`` or an object of error type ``E``. The latter can be used to obtain additional information about the error, e.g. an error code.
In a way this extends error codes and may act as kind of a replacement of exceptions. It is usually used as return type of functions which may fail for various reasons and should be used if the error is supposed to be delegated to the caller and handled in the caller context.
It is also possible to further propagate the error to the next function in the call-stack (since this must be done explicitly, which is comparable to rethrowing an exception).

It is possible to use the ``nodiscard`` option to force the user to handle the returned ``iox::expected``.
If the error cannot be handled at a higher level by the caller, the error handler needs to be used.

Examples include wrapping third party API functions that return error codes or obtaining a value from a container when this can fail for some reason (e.g. container is empty). If no additional information about the error is available or required, ``iox::optional<T>`` can be used instead.

### Error handling in posh

Error logging shall be done by the logger only, no calls to ``std::cerr`` or similar should be performed.

All the methods presented (``iox::expected``, ``IOX_EXPECTS`` and ``IOX_ENSURES`` and the error handler) can be used in posh. The appropriate way depends on the type of error scenario (cf. the respective sections for examples). The error handler should be considered the last option.

### Error handling in hoofs

Console output should be printed exclusively via the iceoryx logger with `LogDebug()`, `LogError()` etc.
Some files may still use `std::cout`/`std::cerr`/`std::clog` but this should be avoided.

The error handler cannot be used in hoofs.

Whether it is appropriate to use ``iox::expected`` even if STL compatibility is broken by doing so depends on the circumstances and needs to be decided on a case-by-case basis. If the function has no STL counterpart ``iox::expected`` can be used freely to communicate potential failure to the caller.

It should be noted that since currently `IOX_EXPECTS` and `IOX_ENSURES` are active at release mode, prolific usage of these will incur a runtime cost. Since this is likely to change in the future, it is still advised to use them to document the developer's intentions.

Do not use `std::terminate` directly but use assert-like constructs such as ``IOX_EXPECTS`` or ``IOX_ENSURES``.

### Interface for 3rd party code

The Error handler as well as logger shall be able to use or redirect to 3rd party error handling or logging libraries in the future. Currently this is neither fully supported nor used.

## Usage

### Logger

The logger can be used similar to the streams in the C++ standard API.
To select the log level, the corresponding logger has to be used, e.g. ``LogErr``, ``LogWarn`` etc.

```cpp
LogWarn() << "log message " << someValue << "log message continued";
```

A line break is inserted implicitly at the end (after "log message continued" in the example).

### Error handler

The most general use case is the following

```cpp
if(noError()) {
    //handle the regular case
} else {
    errorHandler(Error::SOME_ERROR_CODE, ErrorLevel::SEVERE);
}
```

If no error level are specified, the error level is assumed to be FATAL by default.

```cpp
errorHandler(Error::SOME_ERROR_CODE)
```

More information on how to setup and use the `errorHandler` can be found in the API reference.

### `IOX_EXPECTS` and `IOX_ENSURES`

Assume myAlgorithm is part of an inner API and not supposed to be called with a ``nullptr``. We may have used a reference here, this is just for illustration.
In addition the value pointed to is assumed to be in the range (-1024, 1024). While we could check this every time, this can be avoided if we specify that the caller is responsible to ensure that these conditions hold.

```cpp
int32_t myAlgorithm(int32_t* ptr) {
    IOX_EXPECTS(ptr!=nullptr);
    //observe the order, we only dereference after the nullptr check
    IOX_EXPECTS(*ptr > -1024 && *ptr < 1024);

    int32_t intermediate = timesTwo(*ptr);
    //this may not be necessary here to ensure that the next function call is valid,
    //but it states our expectations clearly
    IOX_ENSURES(intermediate % 2 == 0);

    int32_t result = abs(intermediate);

    IOX_ENSURES(result % 2 == 0);
    IOX_ENSURES(result >= 0);
    IOX_ENSURES(result < 2048);

    return result;
}
```

Note that in the case of ``nullptr`` checks it is also an option to use references in arguments (or ``iox::not_null`` if it is supposed to be stored since references are not copyable). It should be considered that ``iox::not_null`` incurs a runtime cost, which may be undesirable.
When IOX_EXPECTS and IOX_ENSURES are implemented to leave no trace in release mode, we do not incur a runtime cost using them. For this reason, it is advised to use them to document and verify assumptions where appropriate.

### `expected`

This example checks the arguments and if they are valid proceeds to compute a result and returns it.
Otherwise it creates an Error object from an errorCode and returns it.

```cpp
expected<SomeType, Error> func(Arg arg) {
    int32_t errorCode = checkArg(arg);
    if(isNoError(errorCode)) {
        SomeType result = computeResult(arg);
        // optionally do something with result
        return result;
    }
    return Error(errorCode);
}
```

The caller is responsible for handling (or propagating) the error.

```cpp
auto result = func(arg);
if(result.has_error()) {
    auto& error = result.error();
    //handle or propagate the error
} else {
    auto& value = result.value();
    //proceed by using the value
}
```

Alternatively a functional approach can be used.

```cpp
auto successFunc = [](SomeType& value) {
    //proceed by using the value
};

auto errorFunc = [](Error& error) {
    //handle the error
};

func(arg).and_then(successFunc).or_else(errorFunc);
```

### Testing fatal error

For fatal errors the error handler will terminate the execution of the binary. In order to test these paths the
`iox::testing::IOX_EXPECT_FATAL_FAILURE` function should be used instead of the `EXPECT_DEATH` gTest macro.
The `EXPECT_DEATH` gTest macro forks the process which slows down the test execution (especially with the ThreadSanitizer enabled)
and causes issues with running thread. The `IOX_EXPECT_FATAL_FAILURE` registers a temporary error handler and runs the provided
function in a separate thread. When the error handler is called `longjmp` is used to prevent the termination and instead ensures
to gracefully shutdown the thread.

```cpp
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
TEST(MyTest, valueOnNulloptIsFatal) {
    iox::optional<bool> sut;
    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.value(); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}
```

In cases where the `IOX_EXPECT_FATAL_FAILURE` cannot be used, e.g. testing the `PoshRuntime` with its singleton, `EXPECT_DEATH`
shall be used with `GTEST_FLAG(death_test_style) = "threadsafe";`.

```cpp
GTEST_FLAG(death_test_style) = "threadsafe";
EXPECT_DEATH(bad_call(), ".*");
```

## Open points

### Centralized error handling

It may be desirable to have a centralized error handling instance where runtime errors on application side are logged and (maybe) handled.
This could also be done in RouDi (by sending information to RouDi), but RouDi already has too much responsibility. Preferably this should be done by a separate application with this sole purpose.
If the application cannot reach the central handler, it shall try to handle the error locally if possible (at least log it).

However, it might be too slow if this would rely on error transmission and responses. If this is to be implemented, the exact mechanism has to be decided on.

### 3rd party error handling

We need to decide how to provide an interface for 3rd party error handling, especially for the runtime. This interface will probably rely on hooks. The signature and callsites of these need to be discussed.
This is related to centralized error handling as well.

### Overriding specific error reaction

* Do we want to provide the ability to override error reaction based on e.g. error codes?
* Do we want to disable certain error levels? (if so, this should preferably happen at compile
time with no or few runtime artifacts). It could be an option to e.g. disable all ``MODERATE`` error reaction at compile time.
* It is probably not reasonable to allow disabling reaction on fatal errors.

This is also related to the hooks for 3rd party error handling we may want to provide.

### Return in case of fatal error

The reporting code does not need to be able to continue properly in case of a fatal error, but there needs to be a return after the error handler call. While the error handler is not required to return, it still might under certain circumstances (e.g. a mock error handler in a test case).

The (complete) intended behavior of the error handler requires some further clarification, especially in the case of fatal errors. In the case of non-fatal errors the code invoking the error handler must be able to continue after the error handler returns.

### Error handling vs. logging

Does it make sense to have ``LogErr`` without an error handler call? If an error occurs it should probably be enforced that the handler is called and not just lead to a log entry.
One reason for this is that currently it is not possible to provide an additional error message to the error handler.

### Additional error information

It would be desirable to allow the possibility to provide additional messages (or even general functions/arguments) to the error handler, which is currently missing.
This can be combined with the addition of error location to the error handler.

An optional stack-trace (at least in debug mode) may also prove very useful.
What is needed to have a limited stack-trace even in release mode?

### Debug vs. release mode

We need to further clarify behavior in release and debug mode of the error handler and ``IOX_EXPECTS`` and ``IOX_ENSURES`` (and maybe the logger as well). Can we have a release build with additional information? (e.g. symbols for a stack-trace).

### Assert

Do we want an Assert in addition to ``IOX_EXPECTS`` and ``IOX_ENSURES``? If so, shall it possibly be active in release mode or only debug mode?

In principle with a sufficiently powerful Assert or ``IOX_EXPECTS`` (resp. ``IOX_ENSURES``), this should not be needed (they are equivalent in their functionality).

## Future improvements

In this section we briefly describe ways to potentially improve or extend functionality of existing error handling components. This list is by no means exhaustive and all suggestions are up for discussion and may be refined further.

### Logger

1. The logger could be extended to include logging over a network to a remote server or similar.
1. Support asynchronous logging.

### Error handler

1. Allow customization for ``MODERATE`` and ``SEVERE`` errors to continue according to a user defined configuration.
1. Add file, line and function information (using ``__FILE__``, ``__LINE__`` and ``__func__``). This would require using macros for the error handler call in a future implementation.
1. If deactivation or reduced operation (e.g. not handling ``MODERATE`` errors) is desired, this partial deactivation should cause no (or at least very little) runtime overhead in the deactivated cases.

### `IOX_EXPECTS` and `IOX_ENSURES`

Allow deactivation in release mode, but it should still be possible to leave them active in release mode as well if desired. Deactivation in debug mode can also be considered but is less critical. Deactivation should eliminate all runtime overhead (i.e. condition evaluation).

### expected

1. Consider renaming ``expected`` to ``cxx::result``, which is more in line with languages such as Rust and conveys the meaning more clearly.
1. Add ``has_value`` (in addition to ``has_error``) for consistency with ``optional``.
1. Improve the monadic error handling (beyond ``and_then``, ``or_else``) to allow for better pipelining of multiple consecutive calls (especially in the success case). This requires careful consideration of supported use cases and intended behavior but can reduce control flow code (``if ... else ...``) in error cases considerably.
