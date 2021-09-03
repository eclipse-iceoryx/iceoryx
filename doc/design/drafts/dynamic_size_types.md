# Dynamic Size Support for Zero Copy Types

## Summary

There are multiple obstacles when it comes to use true zero copy with types
whose size is not known at compile time. At the moment a developer can pursue
three approaches.

1. Send via untyped publisher and provide the size at runtime to the publisher.
   But defining message types with multiple dynamic sized containers would be
   a real challenge when they should be constructed inside of this sample.

2. One can perform a worst case estimation at compile time and declare
   a type with container members which fullfil those estimations.
   This could lead to a lot of wasted memory and if the worst case estimations
   have to be adjusted all the applications which are using this type may have
   to be adjusted as well since the capacity is part of the type.

3. Another approach is to use the heap and copy/serialize the data into a sample
   before sending it with iceoryx. This would imply multiple copies since the
   publisher has to copy the data from the container into the sample and every
   subscriber may has to deserialize/copy the data out of the iceoryx sample.

Those obstacles can be overcome when iceoryx containers like `cxx::string`,
`cxx::vector` or `cxx::{forward}list` support custom allocators which allocate
memory in a part of the shared memory which is also in the process space of the
subscriber available.

To accomplish dynamic size types we require:

1. an allocator concept which supports states for allocators
2. have to adjust the containers like `cxx::string`, `cxx::vector` and 
   `cxx::{forward}list`
3. the publisher has to provide access to the shared memory allocator via the
   sample

### Allocator Type Does Not Effect Container Type

One important restriction for the allocator concept is that the specification
of the allocator does not change the type like the STL C++ allocator concept
does. Function developers for instance do not want to restrict their functions
to shared memory allocated types which would result in some implementation
overhead. One example could be that someone analyses an image with a function
like
`void analyseImage(cxx::vector<int, SharedMemoryAllocator> & a)`. This function
can now be only used with a vector which is using the `SharedMemoryAllocator`.
One could define the allocator type as a template but this would increase the
compile time and can make it nearly impossible to write proprietary code since
templates require the implementation in the header. Furthermore, the function
developer has to be aware of the implementation detail that there is something
like a `SharedMemoryAllocator`.

## Memory Layout

In the following section we would like to send the structure 

```cpp
struct MyData {
  int         value;
  vector<int> image;
  string      text;
};
```

and observe how this is stored currently in the shared memory of iceoryx and
compare it to the suggested solution.

An iceoryx container memory structure looks mostly like this:
```
vector/string {
  mgmt; // contains internal variables to handle the logic and later the
        // suggested allocator

  data; // contains the user data which is stored in the container
}
```

### Static Layout

When we require a sample with our publisher and write `MyData` we acquire a
chunk where all the contents are stored inside one chunk. Hereby is the
Memory Manager the class which handles the distribution of shared memory chunks.
To support different sizes the Memory Manager uses bucket allocators with
preconfigured bucket sizes called Memory Pool.

```
+--------------------- Memory Manager -------------------+
|  Memory Pool with chunk size = 1000                    |
|  +----------------+----------------+----------------+  |
|  |  value         |                |                |  |
|  |  image::mgmt   |                |                |  |
|  |  image::data   |                |                |  |
|  |  text::mgmt    |                |                |  |
|  |  text::data    |                |                |  |
|  +----------------+----------------+----------------+  |
|                                                        |
|  Memory Pool with chunk size = 100                     |
|  +---+---+---+---+---+---+---+---+---+---+---+---+---+ |
|  |   |   |   |   |   |   |   |   |   |   |   |   |   | |
|  |   |   |   |   |   |   |   |   |   |   |   |   |   | |
|  |   |   |   |   |   |   |   |   |   |   |   |   |   | |
|  +---+---+---+---+---+---+---+---+---+---+---+---+---+ |
+--------------------------------------------------------+
```

### Layout With Dynamic Sized Types

With dynamic sized types we move the data part outside of the container and
add an allocator to the container which allows the container to acquire
memory for their data part. When we assume that `MyData::image` and
`MyData::text` require 100 bytes for their data part the memory layout could
look like this.

```
+--------------------- Memory Manager -------------------+
|  MemPool with chunk size = 1000                        |
|  +----------------+----------------+----------------+  |
|  |  value         |                |                |  |
|  |  image::mgmt   |                |                |  |
|  |  text::mgmt    |                |                |  |
|  +-------------+--+----------------+----------------+  |
|                |                                       |
|                +-------------------+-----------+       |
|  MemPool with chunk size = 100     |           |       |
|  +---+---+---+---+---+---+---+---+-+-+---+---+-+-+---+ |
|  |   |   |   |   |   |   |   |   |img|   |   |txt|   | |
|  |   |   |   |   |   |   |   |   |dat|   |   |dat|   | |
|  |   |   |   |   |   |   |   |   |   |   |   |   |   | |
|  +---+---+---+---+---+---+---+---+---+---+---+---+---+ |
+--------------------------------------------------------+
```

This does not require any adaptions on the subscriber side since the subscriber
already mapped this memory region which is handled by the memory manager into 
its local process.

## Design Allocator

The allocator should be designed in a manner so that it can be used inside the
shared memory in an IPC context. This means the design has to consider that
vtables, virtual and inheritance are not allowed as well as function pointers
or the use of `cxx::function` or `cxx::function_ref`.

Therefore this allocator concept is less flexible and more complex as it would
be when everything would run in the same process since we have to use relative
pointer and have to allocate everything on the stack.

```
  +------------------------------+
  |  AllocatorError [enum class] |
  |                              |
  |     # OUT_OF_MEMORY          |
  |     # MEMORY_SIZE_TOO_LARGE  |
  +------------------------------+

  +---------------------------------------------------------------+
  |  Allocator [Concept]                                          |
  |                                                               |
  |   - expected<relative_ptr, AllocatorError> allocate(uint64_t) |
  |   - void free(relative_ptr chunk)                             |
  |   - template<AnotherAllocator>                                |
  |     bool isMoveCompatible(const AnotherAllocator& rhs)        |
  +---+--------+--------------------------------------------------+
      |        |
      |  +-----+---------------------------------------------------------+
      |  | RangeAllocator                                                |
      |  |                                                               |
      |  |   - RangeAllocator(relative_ptr start, relative_ptr end)      |
      |  |   - expected<relative_ptr, AllocatorError> allocate(uint64_t) |
      |  |   - void free(relative_ptr chunk)                             |
      |  |   - template<AnotherAllocator>                                |
      |  |     bool isMoveCompatible(const AnotherAllocator & rhs)       |
      |  |                                                               |
      |  |   # relative_ptr m_start                                      |
      |  |   # relative_ptr m_end                                        |
      |  +---------------------------------------------------------------+
      |
   +--+------------------------------------------------------------+
   | HeapAllocator                                                 |
   |                                                               |
   |   - expected<relative_ptr, AllocatorError> allocate(uint64_t) |
   |   - void free(relative_ptr chunk)                             |
   |   - template<AnotherAllocator>                                |
   |     bool isMoveCompatible(const AnotherAllocator & rhs)       |
   +---------------------------------------------------------------+

+--------------------------------+
| template<typename Type>        |
| IsAllocatorTypeTrait           |   Used to verify that AllocatorTypes
|                                +---------------------------------------+
|   static constexpr bool value; |  are compliant with Allocator Concept |
+--------------------------------+                                       |
                                                                         |
               +---------------------------------------------------------+-----+
               | template<AllocatorTypes...>                                   |
               | VariantAllocator                                              |
               |                                                               |
               |   - expected<relative_ptr, AllocatorError> allocate(uint64_t) |
               |   - void free(relative_ptr chunk)                             |
               |   - template<AnotherAllocator>                                |
               |     bool isMoveCompatible(const AnotherAllocator & rhs)       |
               |                                                               |
               |   # relative_ptr m_ptrToAllocator                             |
               +---------------------------------------------------------------+
```
The `Allocator` described in this diagram is similar to a C++20 concept which can
be verified at compiletime without inheritance. The verification can be realized
with a `IsAllocatorTypeTrait` in C++14.
The shared memory and IPC restrictions are forcing us to implement a
VariantAllocator which provides us type independent access to the underlying
allocator like inheritance would provide. Hereby the implementation for
`allocate`, `free` and `isMoveCompatible` is always the same and can be
implemented similar to the `cxx::variant` move and copy operations. This would
make the `VariantAllocator` easier extendable since the user just has to add
any additional allocator as type in the variadic template list. An alternative
way of implementing this `VariantAllocator` would be with a switch statement
in every method which is selecting the correct type. This would lead to a less
extendable `VariantAllocator` since the implementation has to be adjusted with
every new allocator but would may provide the benefit that the `VariantAllocator`
does not require to be a template class anymore.

The `isMoveCompatible` method will be used by every container to verify if
a real move of the underlying structure can be performed or not. This could be
the case for instance with two kinds of heap allocators or when two
RangeAllocators are using the same range. If this is not the case the move
operations will be replaced with an expensive copy operation (fake move).

Furthermore, the suggested zero copy types will only once acquire memory from an
allocator. The reason is that we would like to guarantee zero copy throughout the
usage of the container otherwise some intransparent copies may occur when the
container allocates memory multiple times and requires one contiguous piece
of memory which the allocator may not provide. (See behavior of `realloc` in C).

### Implementation of a C++20 Concept in C++14
The type trait can be implemented via the C++ SFINAE (substition failure is not
an error) principle.
```cpp
template<typename T>
struct IsAllocatorTypeTrait {
    template <typename C, class = void>
    struct HasAllocateMethod : std::false_type {};

    template <typename C>
    struct HasAllocateMethod<
        C, std::void_t<std::enable_if_t<
               std::is_same<decltype(std::declval<C>().allocate(
                                std::declval<uint64_t>())),
                            void>::value,
               void>>> : std::true_type {};

    // test all other methods
    static_assert(HasAllocateMethod<T>::value, 
              "Type is not an allocator, void* allocate(uint64_t) missing");
};
```

## Design of Zero Copy Containers

The design pursues the following goals.

1. The `cxx::vector<T, Capacity>` should not change in its API.
2. All static cxx containers are still owner of all their data. This means the
   memory which the stack allocator manages must be stored inside the cxx
   container class.
3. A function developer should not have to specify the allocator type of a
   container. This means the allocator type is independent of the container type.
   This has to work:
   ```cpp
   void myFunkyFunction(const cxx::vector<int> & bla) {}

   cxx::vector<int, 20> a;
   cxx::vector<int, 30> b;
   cxx::vector<int> c(Allocator::heap);

   myFunkyFunction(a);
   myFunkyFunction(b);
   myFunkyFunction(c);
   ```
4. All containers must be shared memory compatible, this means no vtables and
   virtual. Furthermore the usage of function pointers, `cxx::function` and
   `cxx::function_ref` is forbidden in all containers and their underlying 
   constructs.
5. Before using any cxx container the user as to call `reserve()` once otherwise
   the container will have a default capacity of zero. It is not allowed to
   call `reserve()` multiple times since this could violate the zero copy
   guarantee.
   Stack based containers do not require a `reserve()` call beforehand but
   should provide this call as well for compatibility reasons.

The draft we provide here is using the `cxx::vector` but the techniques
described can be easily applied to the `cxx::string` or other cxx containers.

```
+------------------------------------------------------+
|template<T>                                           |
|vector                                                |
|                                                      |
|  - vector() = default                                |
|  - template<Allocator>                               |
|    vector(Allocator & allocator);                    |
|                                                      |
|  - expected<AllocatorError> reserve(const uint64_t); |
|  - void release();                                   |
|                                                      |
|  // remaining API is unchanged                       |
+------------------------------------------------------+
                          ^
                         /|\
                          |
       +------------------+--------------------------+
       |template<T, Capacity>                        |
       |vector : public vector<Type>                 |
       |                                             |
       |  (#1)                                       |
       |  - vector(const vector<T> &)                |
       |  - vector(vector<T> &&)                     |
       |  - vector& operator=(const vector<T> &)     |
       |  - vector& operator=(vector<T> &&)          |
       |                                             |
       |                                             |
       |  // same API as current cxx::vector         |
       |                                             |
       |  # uint8_t m_data[Capacity]                 |
       |  # RangeAllocator m_allocator               |
       +---------------------------------------------+
```
All non stack based version of cxx containers will not have constructors similar
to the STL. The only provided constructor has one argument which specifies the
type of allocator one would like to use for that container. If no argument is
provided the heap allocator will be used by default.

The constructors and operations defined in (#1) are added to support implicit
conversion from the generic `cxx::vector`. It makes the following operations possible
```cpp
void f(const vector<int, 20> &a) {}

vector<int> v1;
vector<int, 20> v2;

// this works thanks to implicit conversions from vector<int>
f(v1);
v2 = v1;
```
We can use `cxx::Expects` to ensure that the size of `v1` is smaller or equal
than the capacity `v2`. Later we can overload the copy operation with a method
like `perform_copy` which returns the boolean `true` when the copy operation
was successful and `false` when the size of `v1` exceeded the capacity of `v2`.

Since every stack based vector is a child of the more generic vector we can, with
the help of (#1), also assign and use stack based vectors of different sizes
to each other or use them in functions. This allows us to write code like this:
```cpp
void f(const vector<int, 20> &a) {}

vector<int, 10> v1;
vector<int, 20> v2;

// this works since vector<int, 10> is also a vector<int> and can be converted
v2 = v1;
v1 = v2; // this can fail when v1 capacity is smaller then v2.size, previously 
         // it was only possible when v1.capacity < v2.capacity
f(v1);
f(v2);
```

The only restriction are functions which require a stack based vector of a 
specific size as non const reference. This is usally the case when some function
would like to set the contents of a vector.
```cpp
void setVectorContents(vector<int, 20> &a) {}

vector<int, 10> v1;
vector<int> v2;

// both calls will result in a compile time warning
setVectorContents(v1);
setVectorContents(v2);
```

### Usage

```cpp
cxx::vector<int, 20> a;
a.emplace_back(123); // can be used without a preceding reserve call

cxx::vector<int> c(heapAllocator);
// reserve required
c.reserve(42)
     .and_then([&]{ c.emplace_back(891); })
     .or_else([](auto & error){ /* error handling */});

auto sample = publisher.loan();
if ( sample->sharedMemoryVector.reserve(123).has_error() {
   /* error handling */
   return;
}
sample->sharedMemoryVector.emplace_back(someValue);
```


Copy and move operations should be possible without the developer knowing
what kind of underlying allocator is used. Additionally, it should be as efficient
as possible. That means the move operation should be a real move operation
if possible. This will be guaranteed by `isMoveCompatible` provided by the
allocator.
```cpp
cxx::vector<int, 20> a;

cxx::vector<int> b(heapAllocator);
cxx::vector<int> c(heapAllocator);

// isMoveCompatible() will return false since stack allocated memory cannot be moved
// will call release on b and acquires a new fitting chunk with reserve to store
// the contents of a via copy
b = std::move(a); 

// isMoveCompatible() will return true therefore only the internal data pointer
// will be copied
c =  std::move(b);

// isMoveCompatible() will return false
// will call release on a and acquires a new memory chunk, if the compile time
// capacity argument is insufficient it will fail
a = std::move(b);
```
Move can again fail when for instance the size of `c` exceeds the capacity of
`a`. Here we can again provide two operations, the default C++ approach which
could lead to a call to `cxx::Ensures` and then `std::terminate` if the capacity
is insufficient. Additionally, we could introduce a method like `perform_move`
which returns a boolean signaling if it was successful.

## STL containers and zero copy IPC communication

Most dynamic sized STL containers can be used in combination of a custom
allocator. The internal structure of those containers looks similar to the
structure of their `cxx` pendants
```cpp
template<typename T>
class vector {
  //...
  T * memoryStorageOfVectorElements;
};
```
The pointer hereby is an absolut pointer pointing to a piece of memory in the
local virtual address space. Assume we have two processes A and B which would
like to communicate via a `std::vector`.

Further, lets assume process A has a virtual address space [1 - 100] and 
process B [1000 - 1200] (the numbers are representing the memory addresses of
the beginning and end of the virtual address space).
When process A creates a `std::vector` with a shared memory allocator the 
internal `memoryStorageOfVectorElements` pointer may point to an address like
42 which is inside the virtual address space of process A and pointing to the
correct shared memory position from the point of view of process A. When this 
vector is then transmitted to process B the internal `memoryStorageOfVectorElements`
still points to 42 which is no longer in the virtual address space [1000 - 1200]
of process B which would lead to a segmentation fault.

To overcome this restriction we would have to change the internal implementation
of the `std::vector` so that it is inter process capable which would lead to
a new implementation. Additionally, we are not aware if this is the only obstacle
to overcome since a `std::vector` throws exceptions which are not allowed in
our safety concept or think of iterators which are also not interprocess capable.

Therefore the adaptation of the iceoryx hoofs containers seem like the best
option to realize dynamic sized containers.

## Open Questions

1. Is it possible to specialize the `cxx::vector` for POD types like `int`, `float` etc.
   in a way that they will be set via an allocator defined memset method. This would
   allow us to use a GPU allocator inside of the vector in an elegant way.
   ```cpp
   cxx::vector<float> pointCloud(gpuAllocator);
   pointCloud.reserve(100);
   pointCloud.emplace_back(123); // would be stored directly on the gpu

   cxx::vector<float, 100> someOtherPointcloud;
   pointCloud = someOtherPointcloud; // would be copied directly to the gpu 
   ```
2. When the allocator is successfully integrated we can adapt the internal publisher
   and subscriber lists to be dynamic in size and configurable via config file.
   Furthermore, we could optimize the publishers subscriber list to be a little bit 
   more dynamic.
3. When using the heap as default allocator is it possible that a developer by
   accident uses the heap?
   ```cpp
   cxx::vector<float> points;
   points.reserve(1234); // allocated on the heap
   ```
4. Can relative pointer be used standalone like the heap allocator requires.
   This means without an registered runtime and a mapped shared memory partition.
