#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
    //x.store(true, std::memory_order_relaxed);
    x.store(true, std::memory_order_release);
    //std::atomic_thread_fence(std::memory_order_release);//加入屏障
    //y.store(true, std::memory_order_relaxed);
    //y.store(true, std::memory_order_release);
}

void read_y_then_x() {
    //while (!y.load(std::memory_order_relaxed));
    //y.load(std::memory_order_acquire);
    //printf("y = %d\n", y.load());
    //std::atomic_thread_fence(std::memory_order_acquire);//加入屏障
    //if (x.load(std::memory_order_relaxed))
    if (x.load(std::memory_order_acquire))
        ++z;
}

int main() {
    /*
     * 松散操作可以使用屏障来排序
     * 该例子使用屏障
     *      1. 使得y的store跟释放屏障(memory_order_release)类似了，而不是memory_order_relaxed
     *      2. 使得y的load跟获取屏障(memory_order_acquire)类似了，而不是memory_order_relaxed
     *  最终值得y的存储和导入同步了
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


