# Error Handling Design

## Design premises

1. Low runtime overhead
2. Support reaction on fatal errors
3. Can be partially diabled for recoverable errors which causes no runtime overhead
4. Uses iceoryx logger to log error messages
5. Flexible configuration of error levels and error reaction
6. Extensibility - syntactic sugar
7. for e.g. testing purposes (can be set at runtime)

## Platform

- defines error levels (in addition to FATAL)
- defines error handler, default is an empty implementation (does nothing)
- unique
- mandatory, chosen by inclusion of header

## Module

- contains client code (e.g. hoofs)
- unique id
- defines errors to be used in the module only
- errors should be lightweight codes but this is not mandatory
- uses error handling macro API
- multiple modules are supported (e.g. modules may use code from other modules)

## API

If an error is raised it is always logged according to logger settings and then passed on to the handling code defined by the platform.

### Levels

The only level defined by default is `FATAL`. Additional levels can be defined by the platform.

If an error with level `FATAL` is raised, `terminate` is called immediately after the handling code.

### Macros

- `level` is an arbitrary error level defined by the platform
- `error` is an error (code) defined by the module
- `cond` is a condition with a result that can be converted to `bool`

```cpp
IOX_RAISE(level, error)
```

Raise an error and pass it to the handler.

```cpp
IOX_RAISE_IF(cond, level, error)
```

Raise an error if `cond` evaluates to `true` and pass it to the handler.

```cpp
IOX_ASSERT(cond, level)
```

Raise a `FATAL` error if `cond` evaluates to `false` and pass it to the handler.

```cpp
IOX_DEBUG_ASSERT(cond, level)
```

As `IOX_ASSERT` but only active in debug mode.

```cpp
IOX_FATAL(error)
```

Raise a `FATAL` error.

### Syntatic extensions

Any macro allows to provide an addtional custom message which will be logged in case the error occurs.

```cpp
IOX_RAISE(level, error) << "some message " << someVariable;
```

`someVariable` has to support `operator<<` for logging

It is also possible to conditionally call a function `f` in case the error occurs.

```cpp
IOX_RAISE_IF(level, error).IF_RAISED(f, x, y);
```

This can be combined with a message as well.

```cpp
IOX_RAISE_IF(level, error).IF_RAISED(f, x, y) << "some message;
```

### Use cases

1. `IOX_RAISE` used if some (potentially recoverable error) has occured.
2. `IOX_RAISE_IF` used if some (potentially recoverable) error may conditionally occur.
3. `IOX_ASSERT` used if some condition must hold (the alternative is a fatal error). This is not ensured by design.
4. `IOX_FATAL` used if some `FATAL` error has occured.
5. `IOX_DEBUG_ASSERT` used to document some assumption that a condition should hold by design.

## Design

The basic idea is that the macro API generates lightweight proxy objects that relays the error and allows invoking additional operations such as `operator<<` to log custom messages.

In case of error the actual handling is relayed to the handler.

### Proxy

### Error Handler
