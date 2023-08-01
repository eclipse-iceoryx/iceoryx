# Determinism in iceoryx

Deterministic execution is important for many application domains, especially real-time systems.

However, the meaning of determinism differs between domains such as
computer science and real-time systems.

## Deterministic result

The function result only depends on the input and is entirely predictable in the sense that repeated
execution with the same input (and state of the environment) always leads to the same
output.

The state of the environment refers to any global or member variables the function depends on.
In particular it may depend on the current time or a pseudo random number generator state.

This is the (informal) definition used in computer science.

## Deterministic runtime

The runtime of a function is bounded and the function cannot block indefinitely under normal
conditions. Normal conditions mean that the preconditions of the functions hold and the system is
not already in some fatal error state (that may intentionally or unintentionally block), otherwise
the system is in an abnormal state. In an abnormal state no determinism guarantees can be given.

Furthermore it must be guaranteed that the function execution is scheduled by the underlying OS scheduler.

However, while such time-bounds are guaranteed to exist they are usually not known
and depend on hardware and generally the system state (i.e. other processes, context switches etc.).

This definition is mostly used in real-time systems theory and in the
following referred to as time-determinism.

## Influences on time-determinism

1. Thread timing (including benign race conditions)
1. Context switches
1. Branch prediction and speculative execution
1. Caching
1. Exceptions
1. Dynamic memory allocation with a system allocator (`new`, `malloc`)
1. Blocking OS calls
1. Blocking logic or wait logic (e.g. loops in a spinlock)
1. Hardware and general physics

Some of these such as thread-timing may also effect the determinism of the result,
depending on the algorithm (e.g. concurrent queues).

## Deterministic annotation

The `@deterministic` annotation guarantees that:

1. No exceptions are used
1. No blocking OS calls are used unless the purpose of the function is to wait for some event
1. No dynamic memory allocation with the system allocator takes place

It does **NOT** guarantee that:

1. The function is properly scheduled or completes in any defined amount of time
1. The result itself is deterministic (e.g. concurrent lock-free queue push)
1. Any concrete time-bound is met

Except for functions whose purpose is blocking, `@deterministic` guarantees time-determinism in the
above sense. This makes `@deterministic` functions suitable to be used in real-time applications
where a (somewhat) predictable runtime is required.

We only consider public API for annotation, private functions are implementation details and do 
not require annotation (it can be considered optionally where it is useful).

## Conditional guarantees for templates

iceoryx hoofs provides many template classes and functions that depend on user-defined
types `T`. Any determinism guarantee of a function `f` requires that functions defined 
for the template type `T` that are called by `f` must provide sufficient determinism guarantees
themselves.

For example, if `f` calls a member function of `T` that allocates dynamic memory or throws an
exception, this particular instantiation of `f` with `T` provides no determinism guarantees.

In other words, the determinism guarantee is always conditional for templates. 
It is sufficient that the relevant functions of `T` (those that are called by `f`) provide
the determinism guarantee themselves.

Note that for example the `size` function of `vector` does not depend on the template type `T`, i.e.
the set of relevant functions above is empty. In this case the function is unconditionally
`@deterministic`.

## Conditional guarantees for callbacks

For callbacks specified by the user and provided as e.g. lambda expressions, function pointers or
function wrappers such as `cxx::function` or `std::function` the `@deterministic` guarantee is also
conditional.

A function `f` that is `@deterministic` and invokes a callback only provides the determinism 
guarantee if the callback itself is `@deterministic`.

## Limitations

Due to the effects of modern multi-processor CPUs `@deterministic` functions cannot guarantee
hard real-time constraints where a time-bound must always be met. An independent monitoring
mechanism can be used to check whether time bounds are met at runtime (not part of iceoryx).

## Future considerations

Currently this labeling is experimental due to interaction to gain experience with automatic 
validation methods that verify whether a function is indeed `@determinisitic`. 
Since the validation interferes with tests, the labeling is currently limited to a few headers only. 
It will be expanded in the future if it proves useful and feasible.

1. Reevaluate dynamic memory allocation guarantee for POSIX calls.
1. `@deterministic` functions could be more precisely annotated as time-bounded or real-time-safe.
1. Provide more detailed guarantees such as non-blocking or non-allocating.
1. Extend the guarantee for polymorphic classes (similar restrictions as with callbacks).

