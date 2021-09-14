#include "coro.h"

namespace xy::coro {

bool CoroPool::create(CoroFunc func, void* arg) {
    int select_idx = -1;
    for (int i = 0; i < CO_POOL_SIZE; ++i) {
        if (coro_pool[i].state == CoroState::FREE) {
            select_idx = i;
            break;
        }
    }

    if (select_idx < 0) {
        return false;
    }
    
    TCoro* coro_ptr = &coro_pool[select_idx];
    getcontext(&coro_ptr->ctx);
    //修改当前上下文
    coro_ptr->ctx.uc_stack.ss_sp = coro_ptr->stack;
    coro_ptr->ctx.uc_stack.ss_size = sizeof(coro_ptr->stack);
    coro_ptr->ctx.uc_link = &main_ctx; //调用co_resume时设置main_ctx
    coro_ptr->func = func;
    coro_ptr->arg = arg;

    //设置协程上下文
    makecontext(&coro_ptr->ctx, reinterpret_cast<void (*)()>(CO_FUNC_WRAPPER), 1, this);

    //更改当前协程状态
    coro_ptr->state = CoroState::RUNNABLE;

    //加入就绪队列
    _ready_q.push_back(select_idx);

    return true;
}


void CoroPool::yield() {
    if (_running_q.empty()) {
        return;
    }

    int select_idx = _running_q.front();
    TCoro* coro_ptr = &coro_pool[select_idx];

    //修改状态
    coro_ptr->state = CoroState::SUSPEND;
    printf("coroutine [%d] yield, 返回到main\n", select_idx);

    //恢复正在运行的协程
    running_idx = -1;

    //从运行队列中移出
    _running_q.pop_front();
    //加入挂起队列
    _suspend_q.push_back(select_idx);

    //回到主调线程
    swapcontext(&coro_ptr->ctx, &main_ctx);
}

void CoroPool::resume() {
    int select_idx = -1;
    //优先就绪队列
    if (!_ready_q.empty()) {
        select_idx = _ready_q.front();
        //从就绪队列移出
        _ready_q.pop_front();
        //printf("_ready_q pop: %d\n", select_idx);
    } else if (!_suspend_q.empty()) {
        select_idx = _suspend_q.front();
        //从挂起队列移出
        _suspend_q.pop_front();
        //printf("_suspend_q pop: %d\n", select_idx);
    }

    if (select_idx < 0) {
        return;
    }
    
    printf("运行coroutine [%d], before state: %s\n", select_idx, 
            CoroStateStr::to_str(coro_pool[select_idx].state).c_str());
    //加入运行队列
    _running_q.push_back(select_idx);

    if (coro_pool[select_idx].state == CoroState::RUNNABLE) {
        //修改状态
        coro_pool[select_idx].state = CoroState::RUNNING;
        //当前上下文保存在main_ctx, 协程运行结束后返回到main_ctx
        swapcontext(&main_ctx, &coro_pool[select_idx].ctx);
        return;
    } else if (coro_pool[select_idx].state == CoroState::SUSPEND) {
        //修改状态
        coro_pool[select_idx].state = CoroState::RUNNING;
        //直接回到协程上下文
        setcontext(&coro_pool[select_idx].ctx);
        return;
    }
}

bool CO_CREATE(CoroFunc func, void* args) {
    return CoroPool::instance()->create(func, args);
}

void CO_YIELD() {
    CoroPool::instance()->yield();
}

void CO_RESUME() {
    CoroPool::instance()->resume();
}

bool CO_ALL_FINISH() {
    return CoroPool::instance()->is_all_finish();
}

} // namespace xy::coro


