#pragma once
#include <assert.h>
#include <ucontext.h>

#include <list>
#include <memory>

namespace xy::coro {

#define STACK_SIZE 1 * 1024 * 1024  //1M

#ifndef CO_POOL_SIZE
#define CO_POOL_SIZE 8
#endif

using CoroFunc = void (*)(void*);

enum class CoroState {
    FREE, //空闲
    RUNNABLE, //就绪
    RUNNING,  //运行
    SUSPEND //挂起
};

class CoroStateStr
{
public:
    static std::string to_str(const enum CoroState& state) {
        switch(state) {
        case CoroState::FREE:
            return "FREE";
        case CoroState::RUNNABLE:
            return "RUNNABLE";
        case CoroState::RUNNING:
            return "RUNNING";
        case CoroState::SUSPEND:
            return "SUSPEND";
        }
        return "";
    }
};

struct TCoro {
    ucontext_t ctx;
    CoroFunc func;
    void* arg;
    CoroState state = CoroState::FREE;
    char stack[STACK_SIZE];
};

class CoroPool
{
public:
    static CoroPool* instance() {
        static CoroPool coro_pool;
        return &coro_pool;
    }

    ~CoroPool() {}

    bool create(CoroFunc func, void* arg);

    void yield();
    
    void resume();
    
    bool is_all_finish() {
        if (running_idx > 0) {
            return false;
        }

        for (int i = 0; i < CO_POOL_SIZE; ++i) {
            if (coro_pool[i].state != CoroState::FREE) {
                return false;
            }
        }

        return true;
    }

    static void CO_FUNC_WRAPPER(CoroPool* coro_pool_ptr) {
        if (coro_pool_ptr->_running_q.empty()) {
            printf("_running_q is empty\n");
            return;
        }

        //获取运行的协程索引
        int select_idx = coro_pool_ptr->_running_q.front();

        //执行协程绑定的函数
        void* arg = coro_pool_ptr->coro_pool[select_idx].arg;
        coro_pool_ptr->coro_pool[select_idx].func(arg);

        //执行结束，协程恢复状态
        coro_pool_ptr->coro_pool[select_idx].state = CoroState::FREE;
        
        //恢复running_idx
        coro_pool_ptr->running_idx = -1;

        //从运行队列移出
        coro_pool_ptr->_running_q.pop_front();
        assert(coro_pool_ptr->_running_q.empty());
    }

public:
    //调用协程的线程上下文
    ucontext_t main_ctx;

    //正在运行的协程索引
    int running_idx;

    //协程池
    TCoro coro_pool[CO_POOL_SIZE];

    //就绪队列
    std::list<int> _ready_q;

    //运行队列
    std::list<int>_running_q;

    //挂起队列
    std::list<int> _suspend_q;

private:
    CoroPool() : running_idx(-1) {};

    CoroPool(CoroPool& other) = delete;
    CoroPool& operator = (CoroFunc& other) = delete;

};

bool CO_CREATE(CoroFunc func, void* args);

void CO_YIELD();

void CO_RESUME();

bool CO_ALL_FINISH();

} // namespace xy:coro


