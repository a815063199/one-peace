# 一、内存模型的2个原则
## 1. happens-before (发生于之前)
1. 是程序中操作顺序的基本构件,它指定了哪些操作看到其他操作的结果
2. 对于单个线程，它是直观的，如果一个操作排在另一个操作之前，那么该操作就发生于另一个操作之前
3. 如果操作发生在同一条语句中，一般他们之间没有happeds-before关系，因为它们是无序的
4. 具有传递性

## 2. synchronizes-with（于之同步）
1. 一个被适当内存序标记的原子变量X上的写(store)操作，与在同一个线程中对该原子的store操作，或是任意线程(包括本线程)对X的读(load)操作（也包括读-修改-写）同步

# 二、内存序
## 1. memory_order_relaxed
1. 松散顺序: 单线程的同一个变量的操作仍然服从happeds-before关系, 但是对于其他线程没有任何要求
## 2. memory_order_release, memory_order_acquire
1. 获取-释放序列: 释放(release)操作和获取(acquire)操作同步
## 3. memory_order_consume
1. 获取-释放序列-数据依赖
2. 适用于在原子操作载入指向某数据的指针的场合, 通过在载入上使用memory_order_consume, 以及在之前的存储上使用memory_order_release, 可以确保所指向的数据得到正确的同步
## 4. memory_order_acq_rel
## 5. memory_order_seq_cst
1. 顺序一致: 存储(store)肯定先于载入(load)发生
2. 多线程程序的行为，就好像是所有这些操作由单个线程以某种特定的顺序进行执行的一样
3. 所有线程必须看到操作的相同顺序

# 三、原子操作适用的内存序列
## 1. 存储(store)操作
1. memory_order_relaxed
2. memory_order_release
3. memory_order_seq_cst
## 2. 载入(load)操作
1. memory_order_relaxed
2. memory_order_consume
3. memory_order_acquire
4. memory_order_seq_cst
## 3. 读-修改-写(read-modify_wirte)操作
1. memory_order_relaxed
2. memory_order_consume
3. memory_order_acquire
4. memory_order_seq_cst
5. memory_order_release
6. memory_order_acq_rel

# 4. 标准原子整形的操作
1. fetch_add
2. fetch_sub
3. fetch_and
4. fetch_or
5. fetch_xor

# 5. 释放序列(release sequence)
* 如果存储被标记为
    1. memory_order_release
    2. memory_order_acq_rel
    3. memory_order_seq_cst
* 载入被标记为
    1. memory_order_consume
    2. memory_order_acquire
    3. memory_order_seq_cst
* 并且链条中的每个操作都载入由之前操作写入的值，那么该操作链条构成了一个释放序列(release sequence)
* 并且最初的存储和最终的载入是同步(synchronized-with)的

# 6. 内存屏障
* 屏障一般也被称为内存屏障(memory barriers), 它们之所以这样命名，是因为它们在代码中放置了一行代码, 使得特定的操作无法穿越
* 在独立变量上的松散(memory_order_relaxed)操作可以自由地被编译器或者硬件排序, 屏障限制了这一自由



