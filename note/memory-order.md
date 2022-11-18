# Memory Order

c++种memory order是限制同一个线程内指令的重排，本身和多线程没什么关系。但确是为了解决多线程环境下指令重排导致的逻辑错误。
两个很重要的概念：happen-before 和 sync-with

## memory_order_relaxed

仅仅提供atomic原子操作保证，不对指令重排做限制

## memory_order_consume

不允许后面依赖于该atomic变量的读写指令重排到改指令之前

## memory_order_acquire

后面的指令不能跨域原子操作重排到该原子操作之前

## memory_order_release

前面的指令不能跨域原子操作重排到该原子操作之后

## memory_order_acq_rel

前后的指令不能跨越该原子操作进行重排

## memory_order_seq_cst

前后的指令不能跨越该原子操作进行重排（acq_rel），并且所有线程的语句都以全局的内存修改顺序为参照