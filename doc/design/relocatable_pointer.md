# Relocatable and Relative Pointers

When memory is shared between applications and each application maps the shared memory to a different starting address
in its own address space, raw pointers cannot be used anymore. This is due to the fact that a pointer pointing to some
object in shared memory in one address space will in general not point to the same object in another address space (or
even to the shared memory), i.e. the pointer is invalid. In this case, different pointer concepts such as relative and
relocatable pointers must be used. Depending on the shared memory setup and the relationship between pointer and pointee,
there are different use cases which we need to cover.

Both `RelativePointer<T>` and `relocatable_ptr<T>` are template pointer classes that try to retain as much of the normal pointer
syntax as possible while still being valid across address space boundaries. This means that operations like
assignment, dereferencing, comparison and so on work similar to raw pointers. They do not possess reference counting
semantics in this implementation, but extensions for this functionality are possible.

Note that the naming `RelativePointer` in the codebase is currently inconsistent with `relocatable_ptr` (which mimic STL
smart pointers). This will be changed in a refactoring of `RelativePointer`.

## Shared Memory Setup

In general, we have many shared memory segments, each starting at some address and containing a specified number of
bytes. Specifically we distinguish between management and payload segments. The management segment contains metadata
used by RouDi and the runtime to communicate, store common structures such as ports and so on. The payload segment
contains the actual data samples sent by senders in some application to receivers in other applications.

## Relocatable Pointers

When pointer and pointee are located in the same shared memory segment, the lightweight (compared to a relative pointer)
relocatable pointer can be used. That is, we have the following scenario.

Pointer `p` points to object `X` of type `T` and both are stored in shared memory segment `S`.

```txt
Shared Memory S:  p                    X
                  |____________________^

App1          a1  b1                  c1
App2          a2  b2                  c2
```

Let `a1`, `b1`, `c1` and so on be the addresses of segment `S`, pointer `p` and object `X` in application 1 and similarly `a2`, `b2` and
`c2` in application2. If application 2 maps the memory differently they will be shifted by some common offset `d` depending
on the individual memory mapping:

```
a2 = a1 + d, b2 = b1 + d, c2 = c1 + d
```

This is why storing a raw pointer to `X` will not be sufficient, the value `c1` of `p` will not point to `X` in application 2.
However, storing the difference between the location of `p` and `X` will work since it is an invariant in both address
spaces.

Thus, instead of using a raw pointer `p`, we just use a relocatable pointer `relocatable_ptr<T>` pointing to `X`. This pointer
can then be dereferenced in both address spaces and will point to `X`.

Note that if we only use relocatable pointers in this way in complex data structures that rely on pointers (such as
lists and trees) in shared memory, we can in fact copy the whole shared memory segment somewhere else (i.e. *relocate*
it, hence the name) without compromising the data structure integrity. That is, the list elements in the copy correctly
point to their successor elements in the copied memory. This also presupposes that we do not rely on other side effects
of deep copying (i.e. nontrivial copy constructors).

### Construction and Assignment of Relocatable Pointers

Assume we have an object `X` of type `T` in shared memory. Then we can construct relocatable pointers `p` in the *same
segment* (e.g. as members of objects that reside in this segment or via emplacement new at a memory address in this
segment).

Default construct a logical `nullptr`

+ `relocatable_ptr<T> p;`

Construct relocatable pointing to `X`

+ `relocatable_ptr<T> p(&X);`

Assign relocatable to point to `X`

+ `p = &X;`

### Operations of Relocatable Pointers

Most operations for raw pointers also work for relocatable pointers. In particular, a `relocatable_ptr<T>` is compatible
with a raw pointer `T*` in many cases, such as assignment, to provide seamless integration and convenient syntax.

Comparison with raw pointers

+ `if(p != nullptr) { \\do something }`

+ `T* raw = &X; if(p == raw) { \\do something }`

Assignment to raw pointer (resolves the pointer in the current address space)

+ `T* raw = p;`

Explicitly get the raw pointer

+ `T* raw = p.get();`

Dereference

+ `T& t = *p;`

Access members

+ `p->func();`

### Performance of Relocatable Pointers

Since relocatable pointers just measure the distance to the pointee from itself it just requires a number to store this
pointer difference. This number has the same size as a pointer itself, i.e. 64 bit on 64 bit architectures. All
operations use just a few simple arithmetic instructions in addition to pointer dereferencing and hence they do incur
little overhead compared to raw pointers.

## Relative Pointers

When pointer and pointee are located in different shared memory segments, it gets more complicated and relocatable
pointers are no longer appropriate. This happens in our setup with metadata in the management segment referencing data
in the payload segment.

Pointer `rp` is stored in segment `S1` and points to object `X` of type `T` in segment `S2`.

```txt
Shared Memory  S1: rp                 S2:   X
                    |_______________________^

App1           a1  b1                  c1  d1
App2           a2  b2                  c2  d2
```

It is no longer true in general that both segments will be offset by the same difference in App2 and therefore
relocatable pointers are no longer sufficient.

Relative pointers solve this problem by incorporating the information from where they need to measure differences (i.e.
*relative* to the given address). This requires an additional registration mechanism to be used by all applications
where the start addresses and the size of all segments to be used are registered. Since these start address may differ
between applications, each segment is identified by a unique id, which can be provided upon registration by the first
application. In the figure, this means that the starting addresses of both segments (`a1, a2 and c1, c2`) would have to be
registered in both applications.

Once this registration is done, relative pointers can be constructed from raw pointers similar to relocatable pointers.
However, it should be noted that relocating a memory segment will invalidate relative pointers, i.e. relative pointers
are **NOT relocatable**. This is because the registration mechanism cannot be automatically informed about the
copy of a whole segment, such a segment would have to be registered on its own (and the original segment deregistered).

### Registration

Register the start pointer `start1` of a segment with a specific size (this happens once after the memory is mapped)

+ `auto id = registerPtr(start1, size);`

Register the start pointer of the segment `start2` in another application with a specific id (again, this happens after
the other application maps the memory)

+ `registerPtr(id, start2, size);`

Unregister specific id (also invalidates relative pointers using this id, they cannot be used safely after this
operation anymore)

+ `unregisterPtr(id);`

Unregister all ids (also invalidates all relative pointers)

+ `unregisterAll();`

### Construction and Assignment of Relative Pointers

Assume we have an object `X` of type `T` in shared memory. Then we can construct relative pointers `rp` pointing to *arbitrary
registered segments*.

Default construct a logical `nullptr`

+ `RelativePointer<T> rp;`

Construct relative pointer pointing to `X` (provided the segment containing `X` was registered)

+ `RelativePointer<T> rp(&X);`

Assign relative pointer to point to `X`

+ `rp = &X;`

Copy construction

+ `RelativePointer<T> rp2(rp);`

Copy assignment

+ `RelativePointer<T> rp2 = rp;`

### Operations of Relative Pointers

As for relocatable pointers, operations for raw pointers also work for relative pointers.

comparison with raw pointers

+ `if(rp != nullptr) { \\do something }`

+ `T* raw = &X; if(p == raw) { \\do something }`

Assignment to raw (resolves the pointer in the current address space)

+ `T* raw = rp;`

Dereference

+ `T& t = *p;`

Implicitly get the raw pointer

+ `T* raw = rp;`

Explicitly get the raw pointer

+ `T* raw = rp.get();`

Access members

+ `p->func();`

### Performance of Relative Pointers

A relative pointer measures the distance to the pointee relatively to a registered address and therefore needs to store two
numbers, one for the distance itself and the other one to identify the segment base address to measure from. For this reason,
the operations incur slightly more overhead compared to relocatable pointers.

## Atomic usage

There is a technical problem using both `relocatable_ptr` and `RelativePointer` as a type in a `iox::concurrent::Atomic`.
This is essentially impossible as an atomic requires its type to be copyable/movable (to be loaded) but on the other hand,
this copy constructor must be trivial, i.e. performable with a (shallow) memcpy. Therefore, the types used in atomic cannot implement
custom copy/move. This is not possible for `relocatable_ptr` and `RelativePointer` as both require operations performed
during copying, which cannot be done by a simple shallow copy.

To support the use case of an atomic relocatable pointer, the template `AtomicRelocatablePointer<T>` is provided. It is a
basic version of a relocatable_ptr, which lacks some of its move functionality and other operations. However, its
existing operations behave like `relocatable_ptr` but in an atomic way (but also incur additional overhead caused by
atomic loads and stores). Therefore, this object can be used concurrently by multiple threads, in contrast to
`RelativePointer` and `relocatable_ptr`. This atomic version also utilizes lock-free atomic operations internally (when
available on the target architecture) as the data to be stored is just a 64 bit word.

Note that this cannot be done as easily for `RelativePointer` as it requires more than 64 bit to store its internal
information. A construction where atomicity is guaranteed via locking could be provided in the future.
