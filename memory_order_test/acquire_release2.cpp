#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);

    y.store(true, std::memory_order_release);
}

void read_y_then_x() {
    while (!y.load(std::memory_order_acquire));
    if (x.load(std::memory_order_relaxed))
        ++z;
}

int main() {
    /*
     * 获取-释放操作可以在松散操作中施加顺序
     */
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    printf("z = %d\n", z.load());
    assert(z.load() != 0);
}


