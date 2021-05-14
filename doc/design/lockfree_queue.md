Remark: This is a preliminary description and will be reworked when the capacity feature is implemented.

# Lock Free queue

We explain some details of the lock free queue in order to provide an intuition for the way the lock free queue works. This could serve as a basis for a formal proof later if desired.

## Index queue analysis

In the following all numbers are natural numbers and hence bounded unsigned integers in the implementation.

### Capacity and Cycle Length
The index queue has some capacity n and stores indices in the range [0, n[.
The number n is also called cycle length.

Assume n = 4 in this explanation, so we can store the indices 0, 1, 2, 3.

### Index representation
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

### Queue representation
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


### Head and Tail monotonicity
We always insert (push) at Tail and remove (pop) at Head.
Head and Tail both increase strictly monotonic (making ABA problems unlikely).
I.e. a push causes tail to increase by one (causing the cycle to increase after n pushes) and similarly each pop causes Head to increase by one.

### Push operation
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

### Pop operation
Pop reads the value at the index of head and returns it if the cycle matches Heads cycle and a CAS to increase head by 1 succeeds.

```
[ (c,?), (c,y), (c,x), (c-1,?) ]   H=(c,1)     T=(c,3)
```
pop returns y
```
[ (c,?), (c,x), (c,y), (c-1,?) ]   H=(c,2)     T=(c,3)
```

If the cycle is behind Heads cycle the queue is empty (at the time of this check) and nothing is returned.


### Head Tail relationship
Head is always at most Tail, i.e. H <= T. (Technically we only have H - 1 <= T in general, cf. push implementation)

The general situation is one of the following, with c indicating the cycle of the values and Head and Tail.

(i) Head and Tail are at cycle c and ***** marks the region in the array with values logically in the queue.
```
[  c   |*****c*****|   c-1              ]
       H = (c,i)   T= (c,j)
```

(ii) Head is one cycle behind Tail.
```
[******c*****|   c-1     |******c-1*****]
             T = (c,i)   H= (c-1,j)
```

(iii) Empty queue
```
[     c        |            c-1         ]
               T = H = (c,i)
```
### ABA prevention

Note that the monotonicity of Head and Tail combined with suitable CAS make ABA problems not a practical concern.
Moreover, even if we require many bits for the index and thus have few bits for the cycle, we still require a complete uint64
wraparound for an ABA problem to occur (in addition to re-inserting the same value). 

This is simply because if the maximum cycle is small then the cycle length n must be large and
we increase the cycle every n pushes (or pops), but the numbers representing the index increase by one every operation.

### Lock free analysis
Claim:
The queue is operation-wise lock free: pushes cannot block pops arbitrarily and vice versa.
Furthermore, between concurrent pushes and pops one of each type always succeeds in a finite amount of time.

Proof sketch:
- only pushes read/write Tail but pops do not
- pushes modify array values which pop reads and compares (CAS) 
  but we can have at most n pushes before this stops (push constraint in 5.) and then pop will succeed without interference from push
- only pops read/write/CAS Head
- claim follows now from loop/read/write/CAS structure in implementation

Therefore pushes and pops do not interfere to block each other.

However, pushes can block other pushes and pops other pops. 
In particular operation-wise lock freedom implies the queue is lock free.

Note that there is no fairness guarantee. In principle the same push thread or pop thread might always succeed but in practice this is unlikely.
The queue is therefore not wait free.

This could technically be mitigated somewhat with a more complex logic keeping track of operation failures
but seems not to be worth it in practice. (incomplete analysis, further studies required)
