# libzrvan
Libzervan is a set of header-only c++ POC components developed and customized for high-performance, high-throughput computing. 

## Dependency
There is no particular dependency for using this library (except std c++). but for building and running the tests, you need the following tools and libraries:
- Cmake
- gtest ( https://github.com/google/googletest )
- xxHash ( https://github.com/Cyan4973/xxHash )

## Operation system and CPU

These components are only tested, developed and tuned for the Linux environment and Intel CPUs. For other operating systems or other CPU architectures (AMD, for example), the results may differ.

## Build 
As it is a header-only library, there is not any particular process for building. In case that you want to run the unit tests, you need to follow the following instructions.

    git clone https://github.com/mohsenatigh/libzrvan
    cd libzrvan
    mkdir __build
    cd __build
    cmake ..
    make 

## components 

### Hash calculators

    libzrvan::utils::CoreHash

Implementation of core hash algorithm. used to decrease the collision chance in a numeric space
	
    libzrvan::utils::FastHash

Implementation of the Fast hash algorithm. It could be used as a replacement for the std::hash function. In most implementations, std::hash uses MurmurHash. Fast hash is a competitor algorithm that pos better performance especially in short strings (between 2-128 characters). please note that for std based data structures, using this hash algorithm is not recommended.

The following chart compares the performance of fast hash against std::hash and xxHash (https://github.com/Cyan4973/xxHash). Although fast-hash poses better performance, its distribution factor is less than two other hash functions. Anyway it is enough for most situations. 

![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/FastHash.png)

### Counter

    libzrvan::utils::Counter

Atomic variables are typically used for collecting and tracking metrics and statistics. Using this technique may cause unnecessary CPU usages and CPU cache miss in a high throughput multithreaded system.
Operation systems usually use per CPU counters for tracking these types of metrics  (for example, NIC TX RX). The counter class is a simple implementation of the per-thread counter. It is a suitable mechanism for counters that update frequently but read rarely. In exchange for highly optimized write performance, It uses a lot more memory and has a slow read operation.

The following graph compares the performance of per-thread counters with atomic variables. for more details, please check the related unit test     

![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/Counter.png)

### Locks

    libzrvan::utils::SpinLock

Simple spinlock implementation. in low contention situations this lock act as a simple spinlock but in highly contention systems it acts as a sleeping mutex. When it is acting as a sleeping mutex the performance is highly dependent on the OS scheduling algorithm and timer

The following chart compares the performance of the common synchronization mechanism used in Linux with libzrvan::utils::SpinLock

![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/Lock.png)
![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/LockLow.png)
![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/LockNo.png)

    libzrvan::utils::RWSpinLock

Simple RWspinlock implementation. in low contention situations this lock act as a simple spinlock but in highly contention
systems it acts as a sleeping mutex. When it is acting as a sleeping mutex the performance is highly dependent on the OS scheduling algorithm and timer. It also
supports Strong-writer mechanisms. It means we can prioritize writer threads over readers.


### Data structures

    libzrvan::ds::ExpSlotList

Thread-safe slot-link list with expiration capability. like many other tools in
this library, it uses high memory to increase performance. It uses 2 separate lists,
one for storing the key elements and another list for storing actual objects. using this
technique will cause a constant list traversal and search without dependency on the
actuall objects size.

- It is possible to have a duplicate key in one list
- To prevent race conditions and object escape, it is important to access the objects in  the callback functions and don't keep a reference to the stored objects
- This list support TTL (time to live) It means it is possible to remove objects after a defined interval from the last access
- if EXTEND_LIFE_ON_ACCESS equal true the DS extend the lifetime of the object after each access by the defined interval
- The expireCheck routine could be called from another thread

To estimate the performance of this library, the following are the comparison results of ExpSlotList with other common std data structures. for more detailed information, please refer to the related unit tests. Please note that these test results are just for reference, because ExpSlotList was designed for a completely different purpose.

![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/ExpListInsert.png)
![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/ExpListSearch.png)

    libzrvan::ds::ExpMap

Thread-safe hash linked list data structure with the expiration capability. Internally it uses an array of ExpSlotList. so it doesn't use  a big lock for access control

- It is possible to have a duplicate key in one list
- To prevent race conditions and object escape, it is important to access the objects in  the callback functions and don't keep a reference to the stored objects
- This list support TTL (time to live) It means it is possible to remove objects after a defined interval from the last access
- if EXTEND_LIFE_ON_ACCESS equal true the DS extend the lifetime of the object after each access by the defined interval
- The expireCheck routine could be called from another thread
- It supports preloading, to increase insertion speed (in exchange for more memory usage)

