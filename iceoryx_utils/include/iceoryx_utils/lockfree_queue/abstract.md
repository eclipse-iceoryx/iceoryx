How to implement a safe lock-free multi-producer multi-consumer queue in modern C++
==============

***Concurrent algorithms and data structures in C++***

**Author:** Matthias Killat

**Company:** Robert Bosch GmbH

**Department:** CC-AD/ESW1 - Software Engineering and Communication in Distributed Systems for Autonomous Driving

**Email:** matthias.killat2@de.bosch.com

--------------

## Abstract

Multicore processors have become ubiquitous over the last decade and lock-based techniques to synchronize data access such as mutexes and condition variables 
are wide-spread in commercial and academic applications. In contrast, developing lockfree algorithms and data structures is less prevalent since it is usually 
much harder to reason about their correctness. Due to increasing performance requirements of software on embedded devices (e.g. in autonomous driving) there 
has been a surge of interest in robust lockfree algorithms in industry. With the introduction of multithreading primitives in the C++ standard it has become 
possible to implement such algorithms efficiently in a portable way. 

We present a lockfree queue which exhibits FIFO behavior by using an internal ring-buffer and allows an arbitrary number of concurrent producers and consumers 
to enqueue and dequeue data, respectively. This multi-producer multi-consumer (MPMC) queue has a number of desirable other properties which makes it suitable 
to be used in environments with high safety requirements. Most importantly it provides the strong lockfree robustness guarantee that even if some participating 
application threads using the MPMC queue stop responding (e.g. due to a crash or infinite loop) the queue itself remains in a consistent state so that other
threads can still continue normally. Secondly it avoids using dynamic memory which is undesirable if the application is supposed to run for a long time due to
heap fragmentation and the possibility of out-of-memory exceptions compromising availablity of the whole system. It also employs some eviction strategy when 
we are enqueuing into a full queue by simultaneously inserting a new element and returning the oldest element in the internal ring-buffer. 
Finally, enqueue and dequeue operations are not directly competing with each other on shared data, which causes internal compare-and-swap operations to only 
fail rarely in general. We therefore observe only a small performance overhead compared to lock-based implementations in low contention scenarios.

We show how this MPMC queue can be implemented within the C++11 memory model using atomics and compare exchange operations. Key points in the implementation
are using non-blocking ways to mitigate the so-called ABA-problem by using an elaborate internal index structure as well as a technique where operations that 
cannot progress themself help other operations to complete, which in turn allows them to continue with their task.
We also sketch a proof of correctness of the enqueue and dequeue operations and discuss approaches to test the implementation.

This MPMC implementation is used in the Iceoryx open source project (https://projects.eclipse.org/proposals/eclipse-iceoryx) where it admits fast and robust
transmission of large amounts of data. Owing to its properties, the MPMC queue can easily be stored in shared memory which allows it to be accessed by multiple 
independent applications concurrently in order to transmit arbitrary copyable data structures between these applications.

