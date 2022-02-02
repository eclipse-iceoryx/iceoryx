# Prefix Tree

## Problem

We want to search values corresponding to string keys efficiently. The search variants shall include exact and prefix searches,
i.e. find all values where the corresponding key matches a given prefix.

A well-known data structure which solves this problem is a [trie](https://en.wikipedia.org/wiki/Trie), or prefix tree.

## Requirements

### Functional

The `PrefixTree` class shall support

1. string keys only
1. storage of values of a regular type (i.e. copyable, equality comparable and similar mild restrictions)
1. multiple values per key (as in a multi-map)
1. duplicate values per key are allowed
1. insertion of values corresponding to a key - **complexity O(keylength)**
1. removal of a single key and all its corresponding values - **complexity O(keylength)**
1. removal of all values of a given **complexity O(keylength + #removed)**
1. finding all values of a given key **complexity O(keylength + #resultset)**
1. finding all values of all keys containing a given prefix **complexity O(#resultset)**
1. a configurable maximum number (compile time) of values to be inserted

### Additional Properties

1. The `PrefixTree`shall be **relocatable**, i.e. a `PrefixTree` object shall be copyable with memcpy (trivially-copyable).
1. As a consequence `PrefixTree` objects can be transferred over Shared Memory without serialization (zero-copy).

The requirements require some slight modifications to regular prefix trees to support multiple values per key,
ensure relocatability and avoid using dynamic memory.

## Operations

Semantics of the basic tree operations.

### **Insert(key, value)**

Inserts the pair (key, value) in the tree. If the key already exists the value is added to the internal value list associated with the key.

### **remove(key, value)**

Removes in the tree. If the key already exists the value is added to the internal value list associated with the key.
Does not change the tree if the (key, value) pair does not exist. If there is no other value associated with key, the key is removed entirely.

### **remove(key)**

Removes all key and and all associated values from the tree. Does not change the tree if the key does not exist in the tree.

### **result = find(key)**

Find all values associated with a given key and return them as a result. The result will return pointers to the
actual values in the tree for efficiency and to allow modification of values (update operations).

### **result = find(key, value)**

Get the value pointer associated with a given key-value pair. This way we can check for
the existence of (key, value) in the tree and optionally modify the value.

**Remark** While this may lead to dangling pointers if used incorrectly, it is similar to how the STL treats containers
(e.g. `operator[]` or iterators).

### **result = findPrefix(prefix, value)**

Get the value pointers of all values associated with a key with a given prefix.
We also want information about the key of each of those values in contrast to exact queries like **find**
these cannot be inferred from a prefix query.

### Convenience functions

1. get current size of the tree
2. get all keys in the tree
3. get all values in the tree
4. get all keys with their corresponding values

Other functions like his can be added as needed.

## Solution

The general strategy is to create a linked tree structure of specific nodes. There are two types of nodes,
`DataNode`s containing user data and general `Node`s containing meta-data related to efficient search of the keys.

Since we cannot use dynamic allocation, we use our own allocator specific for the task which is guaranteed to support the requested maximum number of values of the tree.

### TypedAllocator

This is a fast bucket allocator which allows allocation and deallocation of memory for a specific class type T
as well as creation and destruction of objects of type T.
The former corresponds to `malloc` and `free` and the latter to `new` and `delete`.

Allocation and deallocation work in constant time, construction and destruction depend on compexity
of the constructor and destructor.
The allocator owns the memory pool it allocates from, is thread-safe, lock-free and relocatable.

Internally it contains a lock-free queue and a buffer of memory blocks for objects of type T.
The queue always contains indices to the free blocks (which can be allocated).

If the allocator runs out of blocks, `allocate` or `create` will simply return `nullptr`
while `deallocate` and `destroy` will never fail but must always be called with pointers previusly obtained by `allocate` or `create`.

Two allocators are used to allocate nodes of one of the two node types and basically replace `new` and `delete`.
The allocators need to be prepared for the worst-case of inserting a maximum number of of values each with keys sharing (almost) no prefixes.

### Reloctable Pointer

Internally we prefer pointers for computation and traversal whenever possible for efficiency.
To ensure the data structure is relocatable, we have to use `relocatable_ptr` whenever we store internal pointers in the structure.
This introduces no space overhead compared to raw pointers but a small additional runtime cost when dererencing the pointer.
The benefit is that we can design the structure almost as if we were using raw pointers.

Since the nodes need to contain pointers to establish the linked tree structure, we have to use `relocatable_ptr` there.
We use the aliases

* `node_ptr_t = relocatable_ptr<Node>`
* `data_node_ptr_t = relocatable_ptr<DataNode>`

for brevity.

### Node

We use a [de la briandais tree](https://www.scirp.org/%28S%28lz5mqp453edsnp55rrgjct55%29%29/reference/referencespapers.aspx?referenceid=1742945)
representation to use as little pointers per node as possible to conserve space. This leads to a little more overhead for traversal, though.

```cpp
struct Node {
    node_ptr_t child{nullptr};
    node_ptr_t sibling{nullptr};
    data_node_ptr_t data{nullptr};
        
    char letter;
}
```

* `letter` the character associated with the node, part of a key
* `child` points to the first child node
* `sibling` points to the first sibling node, i.e. other children on the same level related to the node
* `data` points to potential data, if `nullptr` there is no corresponding value

#### Node Example

```text
Node a
|
|
Node b --> Node c --> Node d
```

* nodes with letters a, b, c, d
* node a the children Node b, c, d.
* Node a has Node b as child (vertical |)
* Node b has Node c as sibling, Node c has Node d as sibling (horizontal -->)
* all pointers not visualized with lines are `nullptr`, i.e. there are no additional children or siblings
* this structure avoids having a fixed number of pointers to children which do not exist in general (i.e. one for each possible `char`)

### DataNode

Each Node can have a corresponding data node, pointed to by `data`. This indicates that the path to this node is a key in the tree.
We can have multiple values per key and we store them in a linked list. `DataNode`s are the nodes of these lists.

```cpp
struct DataNode {
    data_node_ptr_t next{nullptr};
    Value value;
};
```

* `next` pointer to the next `DataNode` associated in the list
* `value` value coresponding to the key of the `Node` this `DataNode` belongs to (can be of arbitrary regular type)

#### DataNode Example

Here the data are integer values 1, 2 and 3.

```text
Node a
|
|
Node b -------> Node c
|               |
--> 1 --> 2     --> 3
```

* the key corresponding to Node a is associated with no value
* the key corresponding to Node b is associated with values 1 and 2
* the key corresponding to Node c is associated with value 3
* `nullptr` is not depicted (no further values)
* this representation means that to find a specific value we have to iterate over the list
(this can be improved by e.g. hashmaps but is only viable if these are dynamic)
* in general the number of values per key is assumed to be low

In the following we chose a shorthand representation like this. The main problem is to find
the corresponding `Node` and then add or remove values to the `DataNode` list as needed.

```text
Node a
|
|
Node b --> Node c
[1, 2]     [3]
```

## Operations on the Tree Structure

Initially the tree only contains a root node which is associated with the empty string key
and also acts as a sentinel value (i.e. the root is never `nullptr`).
The root has as no value initially since the tree is empty, i.e. contains no keys or values.

```text
Root
```

### Insert

Now we insert the key `abc` with value 1. This leads to the creation of three nodes for the individual characters and one
data node for the value.

```text
Root
|
|
Node a
|
|
Node b
|
|
Node c
[ 1 ]
```

Now we insert the key `ab` with value 2. No new nodes have to be created since the prefix `ab` already exists,
we just have to add the value to the corresponding node (Node b).

```text
Root
|
|
Node a
|
|
Node b
| [2]
|
Node c
[1]
```

Similarly we can add the empty string key `""` with value 3. This requires no new node either since the root node always exist.

```text
Root
| [3]
|
Node a
|
|
Node b
| [2]
|
Node c
[1]
```

Now we insert key `aab` with value 4. Here we can reuse the prefix `a` but need new Nodes for the remaining characters.
Also note that we order children lexicographically (which has advantages in ordered enumeration).

```text
Root
| [3]
|
Node a
|
|
Node a --> Node b
|           | [2]
|           |
Node b     Node c
[4]           [1]
```

Finally we add a new value 5 to key `ab`.

```text
Root
| [3]
|
Node a
|
|
Node a --> Node b
|           | [2, 5]
|           |
Node b     Node c
[4]           [1]
```

The resulting tree now contains (in lexicographical order)

1. `""` with value 3
1. `ab` with values 2, 5
1. `aab` with value 4
1. `abc` with value 1

### Find

To locate a key we simply follow the path from the root to the last symbol of the key. If no such path exists,
the key is not in the tree. If the path exists we have to check whether there are actually values associated with the key.
If not, the key is not in the tree (the path is just some prefix of another key in the tree).

Once we have located a key we can access its corresponding values (if any) and add new values or remove or modify existing values.

For example searching for `ab` would yield values 2 and 5, but searching for `a` would yield nothing
since there is no corresponding value. A search for `acb` would also yield nothing since the path does not even exist in the tree.

It is also possible to search for all keys with a given prefix by simply enumerating all values in the subtree
rooted at the end of the path corresponding to the prefix. For prefix `ab` this would yield (`ab`, 2), (`ab`, 5) and (`abc`, 1).

Note that the tree structure does not allow to easily locate substrings or approximate key matches
(this requires other, more complicated approaches and is not needed here).

### Remove

Since we can easily locate keys, it is straightforward to remove the key or one of its values.

For example we can remove the key `b` with all its values.

```text
Root
| [3]
|
Node a
|
|
Node a --> Node b
|           |
|           |
Node b     Node c
[4]           [1]
```

Note that we cannot remove any node since they are still used in prefixes of valid keys.
It is also possible to remove indivudal values of a given key. If the key-value pair in question is not in the tree,
the tree remains unchanged (and an indicator that there was nothing to remove can be returned).

If we now also remove `abc`, the nodes of the suffix `bc` are not used anymore and can be removed entirely.
This could be delayed but is ultimately needed to ensure we have enough nodes for new keys (as they are limited).

```text
Root
| [3]
|
Node a
|
|
Node a
|
|
Node b
[4]
```

Removing `aab` as well leads to the tree which only contains (`""`, 3).

```text
Root
[3]
```

## Relocatability

Relocatability is a property that ensures that we can use memcpy to copy an existing (tree) structure
and the copy is fully functional and in the same state as the original, i.e. the structure is trivially-copyable.
This is not the case for, say, most STL containers where a copy invokes a copy constructor which may have considerable overhead.
For complex structures (e.g. maps) this leads to essentially recreating the structure at the destination
by inserting the elements of the original one by one.

Another consequence is that we can remap the memory (i.e. shift the addresses by some offset) and
the structure will still be operational and in the same state as before. Since this is what happens when the memory is mapped
into a different application, this allows us to share the tree data structure over shared memory without serialization or copies.
In particular, it can be used as data type of an iceoryx publisher or subscriber.

### How to ensure Relocatability

1. Use members that are trivially-copyable.
1. Only refer to memory owned by the object.
1. Only use indices or offsets relative to the start of the object (`this`).

**Remark:** classes are relocatable members are relocatable (trivially-copyable) if and only if
all its members are relocatable. All primitive data types are trivially-copyable.

**Claim:** `PrefixTree` is relocatable.

This follows since we use only `relocatable_ptr`s in the nodes and the remaining members are of type `TypedAllocator`,
which is trivially-copyable. The latter holds since it uses indices internally and owns its memory.

## Memory consumption

Since we need to be prepared for a maximum number of values C, we need to have a sufficient number of nodes to accomodate them.
This is trivial for the data nodes, which we need a most C of as we only store at most C values in the tree.
Each of the nodes just has an extra pointer, which is fairly minor.

It is more problematic for the nodes used for the individual characters, which require 3 pointers for each character stored.
In general there are ways to compress sequences of characters in the tree where no branching occurs,
but this is more complex and not feasible with static memory (how many chars should we store in a node?).

Tightly-estimating there worst case number of thse nodes is harder, since in general we may have keys with little prefix overlap.
By the pigeonhole principle, we will always have some overlap if C is large enough.
For example we only have 256 character values and hence for C > 256 values with different keys
we will have at least one shared prefix (of one character).
This argument becomes increasingly weaker for subsequent levels of the tree.

Disregarding this, a worst case estimate is

```text
#Nodes <= MaxKeyLength * C.
```

This can be imrpoved somewhat for C > 256 to

```text
#Nodes <= (MaxKeyLength-1) * C + 256.
```

which does not help much. Further improved estimates accounting for more upper-level overlap appear not really useful.

This leads to considerable memory consumption for each object, but at runtime this memory is only needed for pathological cases
which have little prefix overlap in the keys and long key strings.

We can introduce another control parameter which lets the user limit the number of these key nodes,
but it is hard to quantify its effects for the user.

If we consider the MaxKeyLength a constant, the memory consumption is still linear in C.

## Generalization

The `TypedAllocator` and `relocatable_ptr` can be ued to create arbitrary relocatable linked structures,
e.g. lists, trees of different kinds like k-d-trees, maps, sets etc.

These can be used in shared memory and transferred with zero-copy overhead (once constructed) using iceoryx.

## Alternatives

### Brute-force

* arrays work well enough up to some point (estimate in 100-300 strings), depending on the searches
* prefix searches are more costly on average but not strictly required.

### Unordered Map

* like `std::unordered::map`
* time complexity crucially depends on a good hash-function and collision handling
* with a good hash-function the performance should be comparable to prefix trees
* relatively space efficient
* no prefix-search

### Maps (Dictionary)

* like `std::map`
* more complicated tree structure (e.g. red black tree) for efficiency
* even harder to implement with static memory constraints
* less efficient search, not linear in keylength but O(#entries in map)
* moderate memory consumption
* limited prefix search


Furthermore multi-map capabilites lead to a lot of wasted storage in all cases if not carefully
handled by value lists (as in the `PrefixTree`).

## Future extensions

1. iterator support
1. functional API
1. generic keys - arrays of ordered (`operataor<`) symbols (currently these are `char`)
1. reduced space consumption (difficult with static memory)
1. make pointer tpye configurable to fall back to raw pointers if relocation is not needed
1. consider whether move is reasonable (would necessarily degenerate a copy with the current design)
1. improve represenation of mutiple values per key to allow faster access (only relevant if number of values per key is high)
1. Do we want to allow duplicate values per key? (i.e. insert and store e.g. 73 twice for a given key?) Currently this is irrelevant.
1. relax the capacity requirements to improve memory consumption
1. return keys of in lexicograpical order for prefix or all key searches (requires a queue in the method call for a breadth first search traversal, maybe not better than sorting afterwards currently)
