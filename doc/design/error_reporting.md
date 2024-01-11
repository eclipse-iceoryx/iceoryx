# Error Reporting API

This is intended as a quick guide through the new error reporting API that will gradually replace the previous error handler.

## Terminology

### Module

Self-contained part of code, e.g. hoofs or posh (or smaller scope). Each module must have a unique
id and defines its errors.

### Module Id

A unique id for each module that is defined by some central instance that knows all modules that are
to be used in some project.

### Custom Implementation

The backend code that is used by any module. It can be changed at compile time and there is only
one active implementation at a time.

### Error Reporting API

The frontend to report errors that is used by the individual modules.

### Error Handler

The part of the custom implementation that defines the actions on reported errors.

### Error Code

Some error code that is unique for each error module. It is recommended that it corresponds only
to the type of error (e.g. out-of-memory error) and not the location.

### Error

An error object to be reported. Must contain at least some error code and is associated with some
module (by its id).

## How It Works

The whole mechanism is very flexible and generic and relies on overloading and perfect forwarding
to relay information, e.g. errors. That means much of it can be optimized at compile time,
as the compiler has full knowledge about all the template definitions.

This holds up to the custom implementation, where it is possible to e.g. use runtime polymorphism or simply link
against some library.

### Reponsibilty of the Custom Implementation

1. Define the error handling backend
1. It expects that errors satisfy some mild conditions (like providing a code)
1. Provide a header that defines the reporting; to be used by any module

In principle a custom implementation can support a wide range of error types, from simple error codes to monadic result
types. The only condition is that the backend must be able to handle them.

In this way, the custom implementation defines a contract for the errors.

### Reponsibilty of a Module

1. Define all its errors and error codes
1. The errors must satisfy the error contract conditions required by the custom implementation.
1. Define how codes are converted to errors
1. Combine the definitions with the API and provide a header that defines its whole error reporting.

Once this is defined, the module can use the error reporting in its own functions.

### Kinds of Errors

There are three mandatory kinds (or categories) of errors.

1. Fatal errors
1. Precondition violations
1. Assumption violations

All of them must be supported by the custom implementation and all of them abort execution (which cannot be
changed).

Furthermore the custom implementation may define additional kinds of errors, specifically non-fatal errors.
For non-fatal errors the execution continues after reporting, so they should be used when the
circumstances allow recovery from the error.

### Panic

Before execution is aborted, a special `panic` handler is invoked. The custom implementation defines this handler
as part of the backend.

## Using the API to Report Errors

Assume the module defines some error codes

```cpp
enum class Code
{
    OutOfMemory = 73,
    OutOfBounds = 21
};
```

and the custom implementation defines a non-fatal `RUNTIME_ERROR`.

The functions described in the following subsections may be used to report errors.

### Panic

```cpp
IOX_PANIC("some message");

IOX_PANIC("");
```

Signals panic, invokes the panic handler and aborts execution.

Even if there is no message to be provided, it has to be called with an empty message. This is a
technical limitation from macro usage (required for location) as it is not allowed to have empty variadic
macros.

### Report a Non-fatal Error

To report a non-fatal error the error code (later to be extended to error objects) has to be provided

```cpp
IOX_REPORT(Code::OutOfMemory, RUNTIME_ERROR);
```

This reports a `OutOfMemory` error and continues execution.

### Report a Fatal Error

Similarly

```cpp
IOX_REPORT_FATAL(Code::OutOfMemory);
```

reports a fatal error that aborts execution after the custom implementation specific handler is invoked.

Decoupling the error and its category is intentional, as e.g. an `OutOfMemory` error may not always be
fatal. It may become fatal after it is propagated further along the call stack.
Propagation is possible by various means, e.g. return codes, monadic types or even exceptions (that
must be caught before reporting the error elsewhere again).

### Conditionally Report an Error

Conditionally reporting an error if some condition holds is useful for more compact error reporting.

```cpp
int x;
// ...
IOX_REPORT_IF(x<0, Code::OutOfBounds, RUNTIME_ERROR);
```

Fatal errors can be conditionally reported in a similar way.

```cpp
IOX_REPORT_FATAL_IF(x<0, Code::OutOfBounds);
```

### Enforce a Condition

Similarly we can conditionally enforce whether a condition does hold and report a fatal error in
the case that it does not hold

```cpp
int x;
// ...
IOX_ENFORCE(x>=0, "enforce violation message");
```

The condition is required to hold and this requirement is always checked.
If the condition does not hold, panic is invoked and the execution stops.

This should be used for conditions that may not hold on the correct path, e.g. for error cases.
It should not be used for assumptions that have to be true in correct code
(use `IOX_ASSERT` for this).

Note that no condition can generally be enforced in the sense that it must be true and no checking is required.

### Assert a Condition

The following check is disabled for release builds and is intended to increase safety during incorrect
use, specifically detect incorrect use at runtime.

This means it should only be used for checks that are not needed in correct code (i.e. defensive
programming).

When check is disabled, there is no overhead in the code, i.e. no checking or reporting takes place.

```cpp
int f(int x)
{
    IOX_ASSERT(x>=0, "precondition violation message");

    // some computation
    int y = g(x);

    IOX_ASSERT(y>=0, "assumption violation message");
    // proceed assuming that y>=0 holds
}
```

It can be used to verify preconditions before any logic in the function body is executed. Technically copy
constructors may run before any condition can be checked, and there is also the possibility of
reordering if the following code does not depend on the condition at all.
This is not a problem since any reordering is not allowed to affect the observable result.
Specifically it cannot affect the value of the precondition itself as this would change the
observable behaviour.

When used in the middle of a function it serves as documentation of assumptions that should hold at this
point in the code before the next statement and can be used e.g. to check for out-of-bounds accesses. It can
also be used to check postconditions.

In case of violation, the violation and a (potentially empty) message are forwarded to the backend,
panic is invoked and execution stops.

## Marking Unreachable Code

It is also possible to explicitly state that code is supposed to be unreachable.
This type of check is always active but does not incur a performance penalty unless the code is
reached.

```cpp
if(condition) {
    // Reachable code that does something
    // This also implies that it is assumed that the condition cannot be false.
} else {
    IOX_UNREACHABLE();
    // Code here should be dead, otherwise it is a bug
    // There should ideally be no dead code, but there are exceptions.
}
```

Another use case is to convey intention by marking exhaustive `switch` statements.

```cpp
enum class Color {
    Red,
    Blue
};

int handleColor(Color color) {

    switch(color) {
        case Color::Red : { return handleRed(); }

        case Color::Blue : { return handleBlue(); }
    }
    // The switch statement is exhaustive and hence this code cannot be reached.
    IOX_UNREACHABLE();
    // No return statement required due to noreturn guarantee.
}
```

If `IOX_UNREACHABLE` is reached during execution, `panic` will be invoked and the program aborts.
Stating that specific code cannot be reached is a specific assumption and any violation
is considered a bug. Defensive programming, i.e. checking for conditions that are not supposed
to happen in a correct implementation, naturally creates unreachable code.

Marking unreachable code like this has advantages for test coverage as the compiler and other tools
that rely on the compiler are aware of the `noreturn` guarantee of `IOX_UNREACHABLE`.
As a consequence, branches with `IOX_UNREACHABLE` do not necessarily lead to a return statement.

### Summary

This shows how the API can be used to either signal errors to an underlying backend or safeguard
against bug conditions such as precodition violations. As the latter should not happen in correct
code, these can be disabled.

## Examples

The following examples show how non-fatal and fatal errors can be signaled.

## Recoverable Errors

The default version only supports error codes.
The following assumes there exists a `RUNTIME_ERROR` category.

```cpp
enum class Code {
    SomeError
};

expected<int, Code> algorithm(int x)
{
    if(errorCondition(x))
    {
        IOX_REPORT(SomeError, RUNTIME_ERROR);
        // control flow continues and the error is propagated to the caller
        return err(SomeError);
    }
    return ok(42);
}

expected<int, E> identity() {

    auto result = algorithm(73);

    if(result.has_error())
    {
        // transform the error to E and propagate it
        return into<detail::err<E>>(result.error());
    }

    // no error, return result
    return ok(*result);
};
```

This is similar to exception handling without the convenience of propagation.
While this shows the use with `expected`, it can be used with any error return type,
for example the error code itself.

A generalization allows to report more complex error types directly. This requires a corresponding
custom implementation.

```cpp
expected<int, Code> algorithm(int x)
{
    if(errorCondition(x))
    {
        // create an exception like custom error
        auto e = err<CustomError>(SomeError, "additional error info");
        // report e directly
        IOX_REPORT(e, RUNTIME_ERROR);
        return e;
    }
    return ok(42);
}

expected<int, E> resultOrError(int x)
{
    auto result = algorithm(x);

    if(result.has_error())
    {
        // transform the error and propagate it
        return into<detail::err<E>>(result.error());
    }
    // no error, return result
    return ok(*result);
};
```

## Non-recoverable Errors

Non-recoverable errors should generally not be used in combination with return codes or other error
types, since the control flow does not return from a fatal error.

```cpp
int algorithm(int x)
{
    if(errorCondition(x))
    {
        IOX_REPORT_FATAL(SomeError);
        // does not return, so no return statement is required
    }
    return 42;
}

int resultOrAbort(int x)
{
    auto result = algorithm(x);

    // if a result was returned, we know that no error has occured
    return result;
};
```

Alternatively the shorthand version can be used

```cpp
int algorithm(int x)
{
    // require that the condition holds or raise a fatal error
    IOX_REPORT_FATAL_IF(errorCondition(x), SomeError);
    return 42;
}
```

A generalization to error types other than codes is possible with a corresponding
backend implementation.

## Structure

### Basics

The overall implementation concepts allow customization of the implementation level and provide
a default implementation.

Everything related to error reporting is located in the corresponding folder `iceoryx_hoofs/reporting/error_reporting`.
Since the main API is stateless, there is no need for classes. Everything directly in this folder
(i.e. not in a subfolder) is not supposed to be changed.

These are

1. `macros.hpp`: the reporting macro API to be used (don't include this directly but only via the specific module header, e.g. `iox/iceoryx_hoofs_error_reporting.hpp`)
2. `configuration.hpp`: the default configuration (compile time flags)
3. `error_forwarding.hpp`: forwarding to the custom implementation
4. `error_kind.hpp`: mandatory error categories
5. `error_logging.hpp`: logging related definitions
6. `source_location.hpp`: source location related definitions
7. `types.hpp`: auxiliary types
8. `errors.hpp` : supported error types and related free functions

Additionally there is the `assertions.hpp` in `iceoryx_hoofs/reporting` which contains the `IOX_PANIC`,
`IOX_UNREACHABLE`, `IOX_ASSERT` and `IOX_ENFORCE` macros.

All the files focus on singular aspects to allow fine-grained inclusion.
All definitions have to reside in `iox::er`, which is considered a private (detail) namespace
for everything related to error reporting. Since the API uses macros, it has no namespace itself.

### Custom Implementation

A specific custom implementation may depend on any of them and has to implement an error reporting
interface `error_reporting.hpp`.

Apart from implementing the error reporting interface, a custom implementation does not have to follow
a specific structure. However, it cannot depend on anything that intends to use the error
reporting itself. This especially is important if e.g. another communication mechanism such as a
socket is used to report the errors. In this case, the socket implementation cannot use the error
reporting as this would create a circular dependency.

A custom implementation can override or extend some definitions and it is encouraged to use the same
file names as in the mandatory basics. For example `custom/error_kind.hpp` specifies additional
error kinds (apart from the mandatory fatal errors).

The main purpose of the custom implementation is to define the actions to take for each error.
Extension of existing definitions is possible by either changing the default implementation
or providing an additional custom implementation and ensure that it is used by all modules.

### Default Implementation

The default implementation in `custom/default` allows switching between a `DefaultHandler`
and a `TestHandler` at runtime.
The latter is used in testing to verify that an error occurred when it is expected.

The `DefaultHandler` is deployed outside of tests and provides minimal logging information.

The default implementation does not depend on any code that uses the error reporting.

### Testing

All testing related definitions are located in `iceoryx_hoofs/testing/error_reporting`.
These are the definition of `TestingErrorHandler` in `testing_error_handler.hpp` and auxiliary
functions in `testing_support.hpp` to be used in tests to verify errors.
The latter can be extended as required.

### Modules

There must be a single point where all modules are defined to ensure they use unique ids and use the
same custom implementation. Currently this happens in the `modules` folder but is work in progress
to be completed during integration of error reporting.

There is `iceoryx_hoofs_error_reporting.hpp` that defines all the errors and custom implementation
used by `iceoryx_hoofs`. This header includes `error_reporting/macros.hpp` to make it easy to use
the custom error reporting in any iceoryx hoofs file by including `iceoryx_hoofs_error_reporting.hpp`.

Replacing the previous error handling is supposed to happen by

1. Adapting the error definitions for `iceoryx_hoofs` in `iceoryx_hoofs_errors.hpp`
2. Introducing a similar folder structure for `iceoryx_posh`
3. Replacing occurrences of the previous error handler call (including `IOX_EXPECTS`)
