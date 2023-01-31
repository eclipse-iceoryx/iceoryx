# Polymorphic Handler

## Goals

1. Manage a singleton handler (the current handler) that inherits from some interface `I`
1. Initialize the handler with some default instance of a class that inherits from `I`
1. Replace the singleton handler at runtime with a singleton of a different class 
that also inherits from `I`
1. Ensure that there is some handler to access at all times (until program termination)
1. Ensure that any accessed singleton handler lives long enough to be accessed by other static
   variables
1. Obtaining the current handler must be thread-safe
1. It must be possible to finalize the handler, i.e. prohibit any changes after it is finalized.
1. Any attempt to change the handler after it is finalized, shall call a function that has access
   to the current and new handler (for e.g. logging).

To achieve this, we define another support class `StaticLifetimeGuard` that solves the singleton lifetime problem.
This class can be used on its own and is based on the nifty counter reference counting.

While obtaining the instance is thread-safe, the instance managed by the handler may not be
thread-safe. If thread-safety of the instances is desired, the classes implementing `I`
must be thread-safe.

## StaticLifetimeGuard

### Properties

1. Manage a singleton instance of some type `T`
1. Lazy initialization on first use
1. Thread-safe instance construction
1. Provide a thread-safe way of obtaining the instance
1. An instance shall only be destroyed after the last existing `StaticLifetimeGuard` object is
   destroyed (regardless of where the `StaticLifetimeGuard` is constructed).

In the following a `StaticLifetimeGuard` is also called guard for brevity.

## Using the StaticLifetimeGuard

### Guard some static singleton instance

```cpp
struct Foo {

};

// create a guard for Foo, note that the instance does not exist yet
static StaticLifetimeGuard<Foo> guard;

// get the instance and store a reference
static Foo& fooInstance = StaticLifetimeGuard<Foo>::instance();

// the fooInstance is guaranteed destroyed after guard is destroyed
// guard could also be held by another static

// alternatively call a static function on the guard (well-defined)
static Foo& sameFooInstance = guard.instance();

// &fooInstance and &sameFooInstance are equal
```

### Manage static singleton lifetime dependencies

```cpp
struct Foo {

};

// Bar uses the fooInstance that is guaranteed to be destroyed
// after guard is destroyed in ~Bar
struct Bar {
    StaticLifetimeGuard<Foo> guard;

    void f() {
        auto& instance = StaticLifetimeGuard<Foo>::instance();
        // use instance
    }
};

// The Foo singleton instance will outlive the Bar instance
static Bar& barInstance = StaticLifetimeGuard<Bar>::instance();

```

This allows creating dependency graphs of static singleton objects. Any static singleton object that
requires another static singleton object simply has to keep a guard of the other singleton object as
a member. The restriction is that it **works with singletons only**, i.e. there can be only one
instance per class that is tracked like this. Hence it is not possible to ensure the lifetime of two
different `Foo` instances.

## StaticLifetimeGuard - implementation considerations

### Instance construction
- thread-safe using atomics only
- during concurrent initialization the initializing call is determined using compare and swap (CAS)
- concurrent access of the instance is delayed until the initializing call completes instance
  initialization
- instance can be accessed like a regular singleton

The construction also creates a `StaticLifetimeGuard` with static lifetime, the primary guard. This ensures basic
static lifetime of the constructed instance (as if it would have been a static variable in a Meyers
singleton itself).

### Reference counting

- global static atomic reference counter (initially zero)
- construction (including copy/move) of each guard object increases the counter by one
- destruction decreases the counter by one
- if the counter reaches zero (last guard destroyed), the instance is  destroyed **IF it was
  constructed before**

### Instance destruction

The instance is only constructed when it is needed (i.e. lazily). There can be several
guard objects without an instance ever being constructed. In this case no instance destruction takes
place once the counter reaches zero.

Due to atomic counter decrement, the instance is destroyed exactly once.

### Instance construction after destruction

It is not possible to replace the instance once constructed due to the static primary guard.
Technically it would be possible by some static destructor after the primary guard (and all other
guards) are destroyed, but this happens only during program termination.

## Using the Polymorphic Handler

### Basic Usage

```cpp
struct Interface {
public:
    virtual void foo() = 0;
};

struct DefaultHandler : public Interface {
    void foo() override { /* ... */ }
};

struct OtherHandler : public Interface {
    void foo() override { /* ... */ }
};

using Handler = PolymorphicHandler<Interface, DefaultHandler>;
```

The first time we access the handler, it is initialized as a DefaultHandler that is guarded 
internally with a `StaticLifeTimeGuard`.

```cpp
// thread 1
// get a reference to the current handler
auto& handler = Handler::get();

// use the handler polymorphically
handler.foo();
```

A concurrent thread 2 may switch the handler to some other handler. This will not interfere with the 
execution of `foo` in thread 1, which is still using the old handler (which is ensured to be alive
by a guard).

The default handler is part of the `PolymorphicHandler`, but any other handler to be set is not. To
ensure the lifetime of handlers that are used, the API to set another handler requires using a
guard. The guard is passed by value and an internal copy ensures any handler being set is not
destroyed before main exits. Any further lifetime must be controlled with external guards.

```cpp
// thread 2
StaticLifetimeGuard<OtherHandler> guard;

// the OtherHandler instance may not be constructed yet

// set the handler to the instance guarded by guard,
// this will create another guard to ensure the lifetime
bool success = Handler::set(guard);
if(!success)
{
    // set may only fail after finalize
    //
    // do something in this case
    // ...
    // even if set was not successful, get() will return the previous handler
}

// OtherHandler instance exists now,
// any other thread will eventually use the new handler

auto& handler = Handler::get();

// unless it was concurrently set again,
// this thread is guaranteed to use the OtherHandler instance
handler.foo();
```

### Lifetime of instances

Any thread using a handler (via `get`) will eventually use the handler that was set last (the order
is determined by atomic set operations).

Holding external references to handlers that were set once remain valid until main exits.
They are not updated to the latest handler on their own, `get` must be used to retrieve the latest
handler.

If externally set handlers are required to live even longer, explicit guards of them must be kept by
other static objects.

### Switching between multiple handlers

When a new handler is set by

```cpp
StaticLifetimeGuard<OtherHandler> guard;
Handler::set(guard);
```
(and the handler is not finalized) the following steps happen

1. Create a guard for the new handler
1. Obtain the new handler instance from the guard
1. Exchange the old handler with the new handler

Afterwards both handlers still exist (they have static lifetime), and using either works if a
reference to it is known. Any threads calling `Handler::get` will use the new handler, but there may
be threads that still use the old handler (as they are currently accessing it and have not called
`get` again). 

Any thread keeps track of its latest known local handler using a `thread_local` variable. This local
handler is initialized the first time the thread uses `Handler`. This is guaranteed to provide the
latest handler instance, but the latest instance can change in the meantime.

The thread can then check whether the local handler is still considered by comparing it to the
global handler (which can be loaded with a relaxed atomic). If both are equal then it is considered
unchanged and the thread will proceed to use the local handler.
Otherwise it will obtain the new handler with a stronger memory synchronization (more costly).

Note that the current handler can change any time but there is no problem as all handlers remain
usbale during the entire program lifetime. Due to this, there are no issues like the ABA problem,
the worst thing that can happen is working with an outdated handler.

This does not require blocking and only relies on fairly cheap atomic operations.
Without using a mutex while using the handler, it is impossible that
threads will always use the latest handler (as it may change at any time).
However, this is not required, it only is required that a handler that is
obtained can be safely accessed. The latter is ensured by using `StaticLifetimeGuard`.

### Thread safety

While obtaining the handler is thread safe and any handler obtained has a guaranteed lifetime until
main exits (or longer, with explicit guards), the individual handlers are not necessarily
thread-safe. This has to be ensured by the implementation of the `Interface`. In particular
`DefaultHandler` and any other handler derived from `Interface` must be thread-safe if it is to be
used by multiple threads.
