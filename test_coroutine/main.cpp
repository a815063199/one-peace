#include <ucontext.h>
#include <stdio.h>
#include "coro.h"
using namespace xy::coro;
 
void func1(void* arg)
{
    int arg1 = *(int*)arg;
    for (int i = 0; i < arg1; ++i) {
        printf("in func1, arg: %d\n", i);
        CO_YIELD();
    }
}

void func2(void* arg)
{
    int arg2 = *(int*)arg;
    for (int i = 0; i < arg2; ++i) {
        printf("in func2, arg: %d\n", i);
        CO_YIELD();
    }
}


void context_test()
{
    char stack[1024*128];
    ucontext_t child,main;
 
    getcontext(&child); //获取当前上下文
    child.uc_stack.ss_sp = stack;//指定栈空间
    child.uc_stack.ss_size = sizeof(stack);//指定栈空间大小
    child.uc_stack.ss_flags = 0;
    //child.uc_link = &main;//设置后继上下文
    child.uc_link = nullptr;//设置后继上下文
 
    makecontext(&child,(void (*)(void))func1,0);//修改上下文指向func1函数
 
    swapcontext(&main,&child);//切换到child上下文，保存当前上下文到main
    puts("main");//如果设置了后继上下文，func1函数指向完后会返回此处
}
 
int main()
{
    //context_test();
    int arg1 = 2;
    int arg2 = 4;
    CO_CREATE(func1, &arg1);
    CO_CREATE(func2, &arg2);
    while (!CO_ALL_FINISH()) {
        //printf("do CO_RESUME\n");
        CO_RESUME();
    }

    printf("at main end, CoroPool finish\n");
 
    return 0;
}
