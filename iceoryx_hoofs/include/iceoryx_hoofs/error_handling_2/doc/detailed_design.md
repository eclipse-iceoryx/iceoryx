# Detailed Design

## Platform

- defines error levels
- defines user specific code called in case of an error
- code depends on type of error and its level

## Application or Library

- consists of modules (at least one)
- modules define error codes
- error codes together with module id define a unique error (system wide)

## Raising an Error

Assume that it is determined at runtime that an error has occured.

The designer has to specify the error level and the code.

Then we raise the error with.

```
IOX_RAISE(level, code)
```

If it is not yet determined whether it is an error and the error depends on some boolean expression `condition`
it is possible to use

```
IOX_RAISE_IF(condition, level, code)
```

Furthermore

```
IOX_ASSERT(condition, code)
```

is equivalent to

```
IOX_RAISE_IF(condition, FATAL, code)
```

### Error raising sequence

Regardless how it is determined to raise the error, once it is raised the following steps occur in sequence.

Remark: sequence diagram can be derived from this

1. invoke macro API with `level` and `code`
2. module id is determined from current module
3. generate `error` object from `level` and `code` (roughly a counterpart of an exception)
4. create `ErrorProxy` object from error and `location`
5. log error information from `error` and `location` depending on `level`
6. report `error` and `location` to the platform specific code
7. execute platform specific code
8. evaluate side effects on ErrorProxy
    - optionally log custom messages
    - optionally execute some function in the current context if there was an error
9. if error `level` was `FATAL`
    - invoke platform specific termination code
    - terminate
10. if error `level` was not `FATAL`
    - destroy `ErrorProxy`
    - continue operation (possibly in error branch)

The platform specific code could e.g. write the `error` to some storage or use another logging framework.
Furthermore the logging call may depend on `level` and be disabled for some levels.

## Entities

### Error Level

- defined by platform
- FATAL always exists

### Error Code

- defined by module

### Error

- created from error code

### ErrorProxy

- created by macro API on error
- uses error, error level, location, log stream API
- uses platform specific reporting API

### Platform reporting API

- uses error, error level, location
- may use third party code
- may implement dynamic error handling (can change at runtime)

## Dynamic error handling

- may be implemented in platform
- define (polymorphic) error handling interface
- interface must specify
    - reaction for each error level
    - reaction on termination
    - implementation can be (partially) empty
- multiple classes can implement the error handling interface
- any object of such a class can be a handler
- at all times there is exactly one such handler installed (singleton)
- handler must be initialized before first use (usually staically)
- dynamic error handling is more costly (dynamic dispatch)

## Error handling during testing

- specific dynamic handler can be set for testing
- testing handler can throw
- this is currently not much of a benefit due to prevallence of `noexcept`
- test handler can also track the errors that occured during a specific code section








