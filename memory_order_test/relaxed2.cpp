#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> x(0), y(0), z(0);
std::atomic<bool> go(false);

unsigned const loop_count = 10;

struct read_values
{
    int x, y, z;
};

read_values values1[loop_count];
read_values values2[loop_count];
read_values values3[loop_count];
read_values values4[loop_count];
read_values values5[loop_count];

void increment(std::atomic<int>* var_to_inc, read_values* values) {
    while (!go)
        std::this_thread::yield();

    for (unsigned i = 0; i < loop_count; ++i) {
        values[i].x = x.load(std::memory_order_relaxed);
        values[i].y = y.load(std::memory_order_relaxed);
        values[i].z = z.load(std::memory_order_relaxed);

        var_to_inc->store(i + 1, std::memory_order_relaxed);

        std::this_thread::yield();
    }
}

void read_vals(read_values* values) {
    while (!go)
        std::this_thread::yield();

    for (unsigned i = 0; i < loop_count; ++i) {
        values[i].x = x.load(std::memory_order_relaxed);
        values[i].y = y.load(std::memory_order_relaxed);
        values[i].z = z.load(std::memory_order_relaxed);
        std::this_thread::yield();
    }
}

void print(read_values* v) {
    for (unsigned i = 0; i < loop_count; ++i) {
        if (i)
            std::cout << ", ";
        std::cout << "(" 
                    << v[i].x << ","
                    << v[i].y << ","
                    << v[i].z << ")";
    }
    std::cout << std::endl;
}

int main() {
    std::thread t1(increment, &x, values1);
    std::thread t2(increment, &y, values2);
    std::thread t3(increment, &z, values3);

    std::thread t4(read_vals, values4);
    std::thread t5(read_vals, values5);

    go = true;

    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();

    /*
     * 打印结果可能如下
        (0,0,1), (1,2,2), (2,3,4), (3,3,4), (4,6,4), (5,8,9), (6,8,9), (7,8,9), (8,8,10), (9,10,10)
        (1,0,1), (1,1,1), (1,2,2), (4,3,4), (4,4,4), (4,5,4), (4,6,4), (5,7,7), (9,8,10), (9,9,10)
        (0,0,0), (1,2,1), (2,2,2), (2,3,3), (5,7,4), (5,7,5), (5,7,6), (5,8,7), (5,8,8), (8,8,9)
        (1,1,1), (3,3,4), (3,3,4), (3,3,4), (3,3,4), (3,3,4), (4,6,4), (5,7,6), (5,7,7), (5,8,8)
        (1,0,1), (2,3,3), (2,3,3), (2,3,3), (2,3,3), (3,3,4), (5,8,8), (5,8,8), (5,8,8), (5,8,8)

        注意到values1, values2, values3 中，由于在单线程中同一个变量存储必须发生在导入之前（内存松散序），
        所以对应的x，y，z列是依次递增的，但是其他2个变量不同，因为不同线程间相对顺序不同。

     */
    print(values1);
    print(values2);
    print(values3);
    print(values4);
    print(values5);

}


