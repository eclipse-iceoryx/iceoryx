# Dynamic Size Support for Zero Copy Types

## Summary

There are multiple obstacles when it comes to use true zero copy with types
whose size is not known at compile time. At the moment a developer can pursue
three approaches.

1. Send via untyped publisher and provide the size at runtime to the publisher.
   But defining message types with multiple dynamic sized containers would be
   a real challenge when they should be constructed inside of this sample.

2. One can perform a worst case estimation at compile time and declare
   a type with container members which fullfil those worst case estimations.
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

To accomplish this we require:

1. an allocator concept
2. have to adjust the containers like `cxx::string`, `cxx::vector` and 
   `cxx::{forward}list`
3. the publisher has to provide access to the shared memory allocator via the
   sample

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

An iceoryx containers memory structure looks mostly like this:
```
vector/string {
  mgmt; // contains internal variables to handle the logic and later the
        // suggested allocator

  data; // contains the user data which is stored in the container
}
```

### Static Layout

When we require a sample with our publisher and write `MyData` we gain have a
chunk where all the contents are stored inside one chunk. Hereby is the
Memory Manager the class which handles the distribution of shared memory chunks.
To support different sizes the Memory Manager uses bucket allocators with
preconfigured bucket sizes called Memory Pool.

```
+--------------------- Memory Manager -------------------+
|  MemPool with chunk size = 1000                        |
|  +----------------+----------------+----------------+  |
|  |  data          |                |                |  |
|  |  image::mgmt   |                |                |  |
|  |  image::data   |                |                |  |
|  |  text::mgmt    |                |                |  |
|  |  text::data    |                |                |  |
|  +----------------+----------------+----------------+  |
|                                                        |
|  MemPool with chunk size = 100                         |
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
|  |  data          |                |                |  |
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

## Design
```
      +----------------------------------+
      | Allocator                        |                     +-----------------------------------+
      |                                  |                     |Publisher                          |
      |  void* allocate(const uint64_t); |                     |  # SharedMemoryAllocator *m_alloc |    // actually ChunkSenderData
      |  void free(void* const chunk);   |                     +-----------------------------------+
      +--+------------------+------------+----+
         |                  |                 |
+--------+-----+ +----------+----------+ +----+--------+
|StackAllocator| |SharedMemoryAllocator| |HeapAllocator|       +--------------------------------+
+--------------+ +---------------------+ +-------------+       |template<Type>                  |
                                                               |Sample                          |
+-------------------------------------------+                  |  Allocator & getShmAllocator();|
|template<Type, Allocator>                  |                  |                                |
|vector                                     |                  |  # Allocator * m_shmAllocator; |
|  vector(Allocator & allocator);           |                  +--------------------------------+
|  void setAllocator(Allocator & allocator);|
|  void reserve(const uint64_t);            |
+-------------------------------------------+

+------------------------------------------------------+
|template<Type, Capacity>                              |
|using vector = vector<Type, StackAllocator<Capacity>; |
+------------------------------------------------------+
```

## Memory Structure
```
struct MyData {                vector<int> {
  int         data;              mgmt; // internal mgmt stuff
  vector<int> image;             data; // user data
}                              }

                                       Shared Memory
old:

+----------------+----------------+----------------+                     --+
|  data          |                |                |                       |
|  image::mgmt   |                |                |   chunk size = 1000   |
|  image::data   |                |                |                       |
+----------------+----------------+----------------+                       |
                                                                           |  inside one memory manager (member of ChunkSenderData)
+---+---+---+---+---+---+---+---+---+---+---+---+---+                      |
|   |   |   |   |   |   |   |   |   |   |   |   |   |                      |
|   |   |   |   |   |   |   |   |   |   |   |   |   |  chunk size = 100    |
|   |   |   |   |   |   |   |   |   |   |   |   |   |                      |
+---+---+---+---+---+---+---+---+---+---+---+---+---+                    --+

--------------------------------------------------------------------------------

new:

+----------------+----------------+----------------+                   ---+
|  data          |                |                |                      |
|  image::mgmt   |                |                |   chunk size = 1000  |
|                |                |                |                      |
+---------+------+----------------+----------------+                      |
          |                                                               |
          +---------------+                                               |
                          |                                               |  inside one memory manager (member of ChunkSenderData)
                          |                                               |
+---+---+---+---+---+---+-+-+---+---+---+---+---+---+                     |
|   |   |   |   |   |   |img|   |   |   |   |   |   |                     |
|   |   |   |   |   |   |dat|   |   |   |   |   |   |  chunk size = 100   |
|   |   |   |   |   |   |   |   |   |   |   |   |   |                     |
+---+---+---+---+---+---+---+---+---+---+---+---+---+                  ---+
```

## Iceoryx Usage

```cpp
struct MyData {
  int         data;
  cxx::vector<int, SharedMemoryAllocator> image;
  cxx::string<SharedMemoryAllocator> text;
  cxx::string<SharedMemoryAllocator> moreText;
}

auto sample = publisher->loan();
// can only be called once
sample->image.setAllocator(publisher->getShmAllocator());
sample->image.reserve(1024);
setImage(sample->image.data());

// can only be called once
sample->text.setAllocator(publisher->getShmAllocator());
sample->text.reserve(11);
sample->text.assign("hello world");

// Alternative API 1:
sample->moreText.reserve(publisher->getShmAllocator().allocate(5 * sizeof(char)));
sample->moreText.assign("hello"); // fails since capacity is zero

// Alternative API 2:
struct MyData {
  // user has to provide custom ctor with shared memory allocator when they 
  // would like to use zero copy types
  MyData(SharedMemoryAllocator & allocator) 
    : image(allocator), text(allocator), moreText(allocator) {}

  int         data;
  cxx::vector<int, SharedMemoryAllocator> image;
  cxx::string<SharedMemoryAllocator> text;
  cxx::string<SharedMemoryAllocator> moreText;
};
```

## ROS2 usage
```cpp
auto loaned_msg = pub->borrow_loaned_msg();
// reserve the size for unbounded sequences
loaned_msg.image.reserve(7500000);
```

## First step

unbounded heap copy to iceoryx serialized

bounded with cxx::vector

# Adjust IDL file with comment only

## Pro
 * IDL files can be reused
 * can optionally be adjusted to fit the content
 * in a complex system with network communication iceoryx can be easily 
   integrated since message types can be reused
 * no API changes

## Con
 * Message transported via DDS to another network instance, which used the
   idl message without capacity comment and has an iceoryx subscriber?
 * Requires runtime string/vector size adjustment or view
 
```
module my_type {
  struct Foo {
    string unbounded_string; // capacity = 128
    string another_string;
  };
};
```

## Howto

 1. Every unbounded type is bounded by a preconfigured limit
 2. IDL compiler can be asked internally to acquire type size 
 3. Extend IDL compiler to parse optional capacity comment
