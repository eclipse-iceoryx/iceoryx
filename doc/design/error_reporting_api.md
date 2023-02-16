# The New Error Reporting API

This is intended as a quick guide through the new error reporting API that will gradually replace
the previous error handler.

## Terminology

### Module 

Self-contained part of code, e.g. hoofs or posh (or smaller scope). Each module must have a unique
id and defines its errors.

### Module Id

A unique id for each module that is defined by some central instance that knows all modules that are
to be used in some project.

### Platform 

The backend code that is used by any module. It can be changed at compile time and there is only
one active platform at a time.

### Error Reporting API

The frontend to repor errors. API for short.

### Error Handler

The backend that defines the actions on reported errors. Defined by the platform.

### Error Code

Some error code that is unique per module.

### Error

An error object to be reported. Must contain at least some error code and is associated with some
module (by its id).

## How it works

The whole mechanism is very flexible and generic and relies on overloading and perfect forwarding 
to relay information, e.g. errors. That means much of it can be optimized at compile time, 
as the compiler has full knowledge about all the template definitions. 

This holds up to the platform, where it is possible to e.g. use runtime polymorphism or simply link
against some library.

## Reponsibilty of the Platform

1. Define the error handling backend
1. It expects that errors satisfy some mild conditions (like providing a code)
1. Provide a header that defines the reporting; to be used by any module

In principle a platform can support a wide range of error types, from simple code to monadic result
types. The only condition is that the backend must be able to handle them.

In this way, the platform defines a contract for the errors.

## Reponsibilty of a Module

1. Define all its errors and error codes
1. The errors must satisfy the error contract conditions required by the platform.
1. Define how codes are converted to errors
1. Combine the definitions with the API and rovide a header that defines its whole error reporting.

Once this is defined, the module can use the error reporting in its own functions.

## Kinds of Errors

There are three mandatory kinds (or categories) of errors.

1. Fatal errors
1. Precondition violations
1. Debug assert violations

All of them must be supported by the platform and all of them abort execution (which cannot be
changed).

Furthermore the platform may define additional kinds of errors, specifically non-fatal errors.
For non-fatal errors the execution continues afer reporting, so they should be used when the
circumstances allow recovery from the error.

## Panic

Before execution is aborted, a special `panic` handler is invoked. The platform defines this handler
as part of the backend.

## Using the API to report errors

Assume the module defines some error codes

```cpp
enum class Code
{    
    OutOfMemory = 73,
    OutOfBounds = 21
};
```

and the platform defines a non-fatal `RUNTIME_ERROR`.

Then we may use the following functions

### Panic

```cpp
IOX_PANIC();

IOX_PANIC("some message");
```

Signals panic, invokes the panic handler and aborts execution.

### Report an error

```cpp
auto e = IOX_ERROR(Code::OutOfMemory);

// ...

IOX_REPORT(e, RUNTIME_ERROR)
```

or, as a shorthand,

```cpp
IOX_REPORT(Code::OutOfMemory, RUNTIME_ERROR);
```

This reports an error and continues execution.

### Report a fatal error

Similarly

```cpp
auto e = IOX_ERROR(Code::OutOfMemory);

// ...

IOX_REPORT_FATAL(e)
```

or

```cpp
IOX_REPORT_FATAL(Code::OutOfMemory);
```

report a fatal error that aborts execution after the platform specific handler is invoked.

Decoupling the error and its category is intentional, as e.g. an `OutOfMemory` error may not always be
fatal. It may become fatal after it is propagated further along the call stack. 
Propagation is possible by various means, e.g. return codes, monadic types or ven exceptions (that
must be caught before reporting the error elsewhere again).

### Conditionally report an error

Conditionally reporting an error if some condition holds is useful for more compact error reporting.

```cpp
int x;
// ...
IOX_REPORT_IF(x<0, Code::OutOfBounds, RUNTIME_ERROR)
```

### Assert that a condition holds

Similarly we can conditionally check whether a condition does not hold and report a fatal error in
the case that it does not

```cpp
int x;
// ...
IOX_ASSERT(x>=0, Code::OutOfBounds)
```

If the condition does not hold, panic is invoked and the executon stops.


## Using the API to check contracts and assumptions

The following checks can be disabled and are intended to increase safety during incorrect
use, specifically detect incorrect use at runtime.

This means they should only be used for checks that are not needed in correct code (i.e. defensive
programming).

If these checks are disabled, there is no overhead in the code, i.e. no checking or reporting takes place.

### Checking preconditions

A precondition check

```cpp
int f(int x)
{
IOX_PRECONDITION(x>=0, "precondition violation message")

// ...
}
```

is used to verify assumptions **BEFORE** any logic in the function body is executed. Technically copy
constructors may run before any condition can be checked, and there is also the possibility of
reordering if the following code does not depend on the condition at all. This is a limitation of
the language and cannot be avoided.

In case of violation, the violation and a (potentially empty) message are forwarded to the backend,
panicis invoked and execution stops.

The verification can be optionally disabled, and hence this also documents assumptions of the
function itself.

### Checking assumptions

Checking assumptions is similar to checking preconditions, but can happen anywhere in the code.

```cpp
int f(int x)
{
IOX_ASSUME(x>=0, "assumption violation message")

// ...
}
```

These serve as documentation of assumptions that should hold at this point in the code before the
next statement and can be used e.g. to check for out of bounds accesses. It can also be used to
check postconditions.

It should not be used at the start of a function body and instead replaced with a precondition check
in this case.

## Summary

This shows how the API can be used to either signal errors to an underlying backend or safeguard
against bug conditions such as precodition violations. As the latter should not happen in correct
code, these can be disabled.
