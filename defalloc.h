// file: defalloc.h
#ifndef DEFALLOC_H
#define DEFALLOC_H

#include <new>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include <algorithm>

/* 注意一点：C++的operator new本身就是可重载的，这是C++多态与C本质上的不同，C语言的内存分配需要用到malloc函数，这个函数本身可以用于C语言，但是它在C中是不能重载的 */

/* 又从编译器的角度对C++静态多态的原理进行了一次复习 
/* C++对于函数的命名原则是：_Z<名称字符个数><函数名称><参数类型列表> */
/* 命名中并不包含返回值，具体原因是什么？ 从函数调用的本身来讲，如果编译器要在编译时(注意到函数重载是静态多态，理论上是在编译期间确定函数调用的类型的) */
/* 而如果我们引入了返回值用于区分两个同名函数，那么考虑两个只有返回值不同的函数： string func1(), int func1()，这两个函数的参数列表都为空，并且调用者在调用的时候没有显式地指明要把返回值赋给一个特定类型的变量 */
/* 那么你会发现此时编译器根本无法仅通过函数签名本身就知道调用者想要的是哪一个函数，考虑到这种可能的错误，C++退而求其次，就不再允许通过函数返回值对同名函数进行区分了 */

template <class T>
/* 签名中这个ptrdiff本身是int类型的别名，别被骗了 */
inline T* allocate(ptrdiff_t size, T*) 
{
    /* 这个函数还会返回原来的placement new处理函数 */
    /* 本身是线程安全的 */
    std::set_new_handler(0);
    /* 常见的C++编译器版本提供两种类型的new操作符重载，一种是placement的，另一种是会调用malloc进行内存分配的，一般不建议用户对placement new操作符进行重载，而对于另一种，用户可以对操作符进行重载以
       自定义申请内存的来源，而不是只服从malloc的安排 */
    T* tmp = (T*)(::operator new((size_t)(size * sizeof(T))));
    if(tmp == 0)
    {
        std::cerr << "out of memory" << std::endl;
        exit(1);
    }
    /* 返回分配好的内存空间的首地址给调用者 */
    return tmp;
}

template <class T>
inline void deallocate(T* buffer)
{
    /* 使用C++提供的默认delete操作函数进行内存的释放 */
    ::operator delete(buffer);
}

template <class T>
class allocator 
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    pointer allocate(size_type n)
    {
        /* 经过观察，个人认为这个pointer(0)传入的作用是供编译器对T的类型进行推导，其值本身并不具有实际的作用 */
        return ::allocate((difference_type)n, (pointer)0);
    }

    void deallocate(pointer p) { ::deallocate(p); }
    pointer address(reference x) { return (pointer)&x; }
    const_pointer const_address(const_reference x)
    {
        return (const_address)&x;
    }
    size_type init_page_size() 
    {
        /* 这里使用临时变量进行大小的判断 */
        return max(size_type(1), size_type(4096/sizeof(T)));
    }
    size_type max_size() const 
    {
        return max(size_type(1), size_type(UINT_MAX/sizeof(T)));
    }
};

// 显式特化版本 (specialization)
class allocator<void> {
public:
    typedef void* pointer;
};
#endif