#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

std::vector<int> queue_data;
std::atomic<int> count;

void populate_queue() {
    unsigned const number_of_items = 20;
    queue_data.clear();
    for (unsigned i = 0; i < number_of_items; ++i) {
        queue_data.push_back(i);
    }
    count.store(number_of_items, std::memory_order_release);//最初的存储
}

void consume_queue_items() {
    while (true) {
        int item_index;
        if ((item_index = count.fetch_sub(1, std::memory_order_acquire)) <= 0)//一个读-修改-写操作
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;
        }
        std::cout << "process " << queue_data[item_index] << std::endl;
    }
}

int main()
{
    /*
     * 由于释放序列(release-sequence)的存在，如果两个线程在读
     * 第二个fetch_sub()将会看到第一个fetch_sub写下的值，而非由最初的store写下的值
     */
    std::thread a(populate_queue);
    std::thread b(consume_queue_items);
    std::thread c(consume_queue_items);
    a.join();
    b.join();
    c.join();
    return 0;
}

