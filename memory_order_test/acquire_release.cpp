#include <iostream>
#include <thread>
#include <atomic>
#include <assert.h>

std::atomic<int> data[5];
//std::atomic<bool> sync1(false), sync2(false);
std::atomic<int> sync(0);

void thread_1() {
    data[0].store(42, std::memory_order_relaxed);
    data[1].store(97, std::memory_order_relaxed);
    data[2].store(17, std::memory_order_relaxed);
    data[3].store(-141, std::memory_order_relaxed);
    data[4].store(2003, std::memory_order_relaxed);
    //sync1.store(true, std::memory_order_release);
    sync.store(1, std::memory_order_release);
}

void thread_2() {
    /*
    while(!sync1.load(std::memory_order_acquire));
    sync2.store(true, std::memory_order_release);
    */

    int expected = 1;
    //执行了存储返回True，否则返回False
    while (!sync.compare_exchange_strong(expected, 2,
                                         std::memory_order_acq_rel));
}

void thread_3() {
    //while (!sync2.load(std::memory_order_acquire));
    int expected = 2;
    while (!sync.compare_exchange_strong(expected, 3, 
                                         std::memory_order_acq_rel));
    assert(data[0].load(std::memory_order_relaxed) == 42);
    assert(data[1].load(std::memory_order_relaxed) == 97);
    assert(data[2].load(std::memory_order_relaxed) == 17);
    assert(data[3].load(std::memory_order_relaxed) == -141);
    assert(data[4].load(std::memory_order_relaxed) == 2003);
}

int main() {
    //获取-释放顺序传递性同步
    std::thread th1(thread_1);
    std::thread th2(thread_2);
    std::thread th3(thread_3);
    th1.join();
    th2.join();
    th3.join();
    return 0;
}
