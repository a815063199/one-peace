#include <atomic>
#include <thread>
#include <assert.h>

bool x = false;//x使用非原子变量
std::atomic<bool> y;
std::atomic<int> z;

void write_x_then_y() {
    x = true;
    std::atomic_thread_fence(std::memory_order_release);//加入屏障
    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed));
    std::atomic_thread_fence(std::memory_order_acquire);//加入屏障
    if (x)
        ++z;
}

int main() {
    /*
     * 在[非]原子操作上强制顺序
     * 该例子使用屏障
     *      1. 使得y的store跟释放屏障(memory_order_release)类似了，而不是memory_order_relaxed
     *      2. 使得y的load跟获取屏障(memory_order_acquire)类似了，而不是memory_order_relaxed
     *  最终使得y的存储和导入同步了
     *  屏障的使用：y.store在释放屏障之后，y.load在获取屏障之前
     */
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    printf("z = %d\n", z.load());
    assert(z.load() != 0);//不会触发
}


