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

Simple spinlock implementation using std::atomic. in low contention situations this lock act as a simple spinlock but in highly contention systems it acts as a sleeping mutex. When it is acting as a sleeping mutex the performance is highly dependent on the OS scheduling algorithm and timer

The following chart compares the performance of the common synchronization mechanism used in Linux with libzrvan::utils::SpinLock

![alt text](https://github.com/mohsenatigh/libzrvan/blob/main/charts/Lock.png)