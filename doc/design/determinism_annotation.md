# Determinism Annotations

## Cert

All functions are to be certified, hence there will be no label to indicate this.

## Deterministic

`\deterministic` indicates that the function is real-time safe, i.e. returns in a finite amount of
time which is (somewhat) predictable.

In particular this rules out.

1. not using exceptions
2. no dynamic allocations from an external allocator (new, malloc)
3. non-blocking

(TBD: Non blocking may still use blocking calls, there must be a return guarantee though.)

Note: deterministic is the wrong wording and should be changed.

## Nondeterminstic

Will not be labeled on its own. Instead we have labels indicating the details.

### Blocking

`\blocking` indicates the code will block and needs o be unblocked externally. Say a mutex lock that
needs to be unlocked from outside the call (it it is unlocked in the call it is not blocking).

A loop that waits for a conition to become true is also blocking if the condition cannot become true
due to actions of the function itself.

### Dynamic Memory
`\allocating` indicates it will allocate memory and implies `\dynamic`
`\dynamic` indicates the code will invoke dynamic memory functions.

Invoking free does count as well. This can be debated, free cannot fail but it will cause an
invocation of the allocator which is unpredictable.

Allocating may also fail.

To clarify: use for all resources (like e.g. semaphores?)

### Throwing

`\throwing`

Can throw internally (independent of client code). This should never be the case. The exceptions can
be thrown and caught internally.

The assumption is that client code (e.g. types are not supposed to throw in certified code)
(otherwise noexcept will become a problem).

## No Annotation

No guarantees.

