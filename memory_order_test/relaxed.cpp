#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);

    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed));
    if (x.load(std::memory_order_relaxed))
        ++z;
}

int main() {
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    /*
     * 可以触发（z可能为0）
     * 松散操作：单线程中的[同一个]变量操作仍然符合happends-before关系，但是
     * 相对于其他线程的顺序几乎没有任何要求；单线程[不同]变量顺序也没要求
     * 可以看到，x和y是不同原子变量，x的导入是可能为false的
     */
    printf("z = %d\n", z.load());
    assert(z.load() != 0);
}


