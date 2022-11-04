# Allocator

## 总体

抽象概念，解耦内存分配各个部分，有点类似于std:pmr的方式：分配策略（具体的Allocator），内存来源（具体的Allocator操作给定的内存来源），多线程同步策略（锁），调试追踪策略（跟踪内存申请情况，过程，用于分析调试内存使用）。每一个部分都可以单独实现，只要实现固定的接口，注册到顶层的模板类中，组合成最终的Allocator。

## STDAllocator 实现，对接STL容器

## 对象池/线程安全的对象池

## VariableSizeAllocationsManager 虚拟内存管理

##