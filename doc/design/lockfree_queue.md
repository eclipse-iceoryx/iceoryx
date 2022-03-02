# Lock-free Queue

We explain some details of the lock-free queue in order to provide an intuition for the way the lock-free queue works. This could serve as a basis for a formal proof later if desired.

A crucial building block to design a lock-free queue for arbitrary types is the lock-free index queue which can only store integer-valued indices.
We briefly describe the idea of the index queue.

## Index Queue

In the following all numbers are natural numbers and hence bounded unsigned integers in the implementation.

### Capacity and Cycle Length

The index queue has some capacity n and stores indices in the range [0, n[.
The number n is also called cycle length.

Assume n = 4 in this explanation, so we can store the indices 0, 1, 2, 3.

### Index Representation

Each index i corresponds to an equivalence class [i] modulo n, i.e.
[i] = {j | j = c*n + i, c >= 0}

This means j = 3, 7, 11, ... all represent the same index 3 (pointing to the last element in the array)

Given j, we have i = j % n and c = j / n (where % is modulo and / integer division).

Another way to represent such a value j uniquely is as a pair (c,i) and
we call this a cyclic index with cycle c and index i.

This is done for two reasons: detection of empty queues and to eliminate the ABA problem for all practical scenarios.
It is imperative that such indices can be used in compare and swap operations (CAS), i.e.
will not exceed 64 bits on standard architectures supporting 64 bit CAS.
Therefore j can be assumed to be an unsigned 64 bit integer.

### Queue Representation

The queue has a head H, tail T and an array of n values. All of them are cyclic indices as in 2.
Initially the queue is empty and head and tail point to index 0 but have both cycle 1.
Array elements are 0 and represented as (0,0).

Initially empty queue

```
[ (0,0), (0,0), (0,0), (0,0) ]   H=(1,0)     T=(1,0)
```

Initially full queue with all indices

```
[ (0,0), (0,1), (0,2), (0,3) ]   H=(0,0)     T=(1,0)
```

### Head and Tail Monotonicity

We always insert (push) at Tail and remove (pop) at Head.
Head and Tail both increase strictly monotonic (making ABA problems unlikely).
I.e. a push causes tail to increase by one (causing the cycle to increase after n pushes) and similarly each pop causes Head to increase by one.

### Push Operation

push(y) causes the cycle at the position Tail points to to be replaced with Tails cycle while the element there is replaced with x
via a CAS operation.
Afterwards Tail is increased by 1 (potentially not immediately, but before the next push takes effect).

```
[ (c,?), (c,x), (c-1,?), (c-1,?) ]   H=(c,1)     T=(c,2)
```

push(y)

```
[ (c,?), (c,x), (c,y), (c-1,?) ]   H=(c,1)     T=(c,3)
```

We only push if the cycle at the element Tail points to is exactly one behind Tails cycle.

Constraint: we can never push more than n elements (and our use case does not require this).

### Pop Operation

Pop reads the value at the index of head and returns it if the cycle matches Heads cycle and a CAS to increase head by 1 succeeds.

```
[ (c,?), (c,y), (c,x), (c-1,?) ]   H=(c,1)     T=(c,3)
```

pop returns y

```
[ (c,?), (c,x), (c,y), (c-1,?) ]   H=(c,2)     T=(c,3)
```

If the cycle is behind Heads cycle the queue is empty (at the time of this check) and nothing is returned.

### Head Tail Relationship

Head is always at most Tail, i.e. H <= T. (Technically we only have H - 1 <= T in general, cf. push implementation)

The general situation is one of the following, with c indicating the cycle of the values and Head and Tail.

1. Head and Tail are at cycle c and ***** marks the region in the array with values logically in the queue.

```
[  c   |*****c*****|   c-1              ]
       H = (c,i)   T= (c,j)
```

2. Head is one cycle behind Tail.

```txt
[******c*****|   c-1     |******c-1*****]
             T = (c,i)   H= (c-1,j)
```

3. Empty queue

```
[     c        |            c-1         ]
               T = H = (c,i)
```

### ABA Problem Prevention

Note that the monotonicity of Head and Tail combined with suitable CAS make ABA problems not a practical concern.
Moreover, even if we require many bits for the index and thus have few bits for the cycle, we still require a complete uint64
wraparound for an ABA problem to occur (in addition to re-inserting the same value).

This is simply because if the maximum cycle is small then the cycle length n must be large and
we increase the cycle every n pushes (or pops), but the numbers representing the index increase by one every operation.

### Lock-free Property

**Claim:** The queue push and pop operations are lock-free.

1. Push operations cannot block pop operaions arbitrarily long and vice versa.
2. Furthermore, between concurrent pushes and pops one of each type always succeeds in a finite amount of time.

**Proof sketch:**

- only pushes read/write Tail but pops do not
- pushes modify array values which pop reads and compares (CAS)
  but we can have at most n pushes before this stops (push constraint in 5.) and then pop will succeed without interference from push
- only pops read/write/CAS Head
- claim follows now from loop/read/write/CAS structure in implementation

Therefore pushes and pops do not interfere to block each other.

Push operatios can block other push operations and pop operations other pop operations due to potential starvation,
but there will always be one operation that will complete (and therefore progress). This progress implies that the queue is lock-free.

Note that there is no fairness guarantee. In principle the same push thread or pop thread might always succeed but in practice this is unlikely.
The queue is therefore not wait free.

This could technically be mitigated somewhat with a more complex logic keeping track of operation failures
but seems not to be worth it in practice. Specifically, the lock-free queue can be made wait-free but with (considerable) decrease
in overall throughput. This can be considered once problems due to starvation are observed.

### Lock-free Queue

A lock-free queue of capacity `N` for a general (regular, copyable) data type `T` uses two such index queues and a buffer to store elements of type T.

- Buffer: N storage cells for values of type T
- Queue 1: free indices - contains the free indices (corresponding cell is unused)
- Queue 2: used indices - contains the used indices (corresponding cell contains data which is considered to be in the queue)

Initially Queue 1 is full and contains all indices `0, ... N-1` and Queue 2 is empty.

We now sketch the main operations in pseudo-code.

#### Lock-free Queue Push

##### push (value)

1. try to obtain a free index i from Queue 1 (pop from free indices)
2. if we got i go to 6.
3. try to obtain an index from Queue 2 **but only if this queue is full** (pop from used indices)
4. if we got no i go to 1. (the queue was **not full**, so there should be a free index)
5. queue overflow, free buffer[i] (evict least recent entry)
6. we obtained a free index i, write buffer[i] = value
7. push i into Queue 2 (used indices)

##### result = pop()

1. try to obtain an index i from Queue 2 (pop from used indices)
2. if we got no i return nothing (queue empty)
3. result = buffer[i]
4. push i into Queue 1 (free indices)
5. return result

#### Lock-free Resize

We can extend these operations with a third one which allows us to set the current capacity even while the queue is in use.
This resize operation allows us to set a new capacity `m` with `0 <= m <= N`.

##### resize(m)

The key idea is that we can use a third container with sufficient lock-free guarantees to store unused indices.
We assume that we use another lock-free index queue for that, but actually a dynamic array (vector) is sufficient as we need no FIFO property.

- Queue 3: unused indices - contains all indices that are currently considered neither free nor used

Initially Queue 3 is empty.

We have to distinguish between decreasing and increasing the capacity. The former can be carried out by moving free indices from Queue 1 (free indices)
to Queue 3. If there is no free index left we pop take used indices from Queue 2 (least recent first), i.e. we discard the least recent elements.
This can be done until the capacity is 0.

The latter simply takes unused indices from Queue 3 (unused indices) and moves them back to Queue 1 (free indices) up to a capacity of `N`.

Since all the involved queues (containers) are lock-free, the resize operation is also lock-free.
