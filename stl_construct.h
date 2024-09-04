#include <new>

struct __true_type { };
struct __false_type { };

/* 小问题：这里的构造参数是否应该使用可变参数列表？ 否则用户如果要实现自己的类型并使用这个包装函数，就必须自己实现一个能够调用只有一个参数的implicit default constructor，这显然不合理 */
template <class T1, class T2>
inline void construct(T1* p, const T2& value)
{
    /* 与常规的new操作符语法进行一下对比 */
    /* new T1(value) */
    /* 不难注意到placement new其实就只是多了一个指定构造指针的信息，没有太多其它区别 */
    /* 个人认为这种调用new的方法和 ::operator new (p) T1(value) 是一致的 */
    new (p) T1(value);          /* placement new; 这一步会在给定的指针位置p上(必须是已经分配好的内存地址)上尝试使用参数value和T1类型的构造函数进行构造 */
    // 会调用 T1::T1(value)
}

// destroy的第一版本 
template <class T>
inline void destroy(T* pointer)
{
    /* 说来惭愧，居然一开始有点怀疑这个写法，毕竟C++类的析构函数总是没有参数的... */
    pointer->~T();              
    // 会调用 dtor ~T()
}

// destroy的第二版本
template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
    /* 书上有说明这种双下划线开头的函数是每个STL实现内部自己使用的函数，并不服从STL的正常规范，所以这里外面套一个无下划线的destroy壳，用来和正常STL标准进行统一 */
    /* 这个函数在SGI版本的STL中额外实现了一种新功能：使用value_type信息从而动态判断要如何执行对象的析构操作 */
    __destroy(first, last, value_type(first));
}

// 判断元素的数值型别 (value type) 是否有 trivial destructor
template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*)
{
    /* 个人认为这里是typename和class之间仅有的几个区别之一了，这里我们假设这里没有使用typename显式地指定后面的__type_traits<T>::has_trivial_destructor本身是一种类型名，那么考虑到编译器在检查模板定义的时候的状态 */
    /* 由于T这个东西只有在运行时才会被确认，而编译器检查这个模板定义的时候，显然不可能有对应模板的实例被创建，因而会导致编译器不知道这个__type_traits<T>::has_trivial_destructor的实际类型(不完整) */
    /* 编译器可能会认为这东西是：1. 类成员变量；2. 类成员函数；3. 类型名 */
    /* 而这里使用typename进行了修饰之后，编译器就知道要typedef的是一个类型名了,这样就消除了歧义 */
    typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
    __destroy_aux(first, last, trivial_destructor());
}

// 如果元素的数值型别 (value type) 有non-trivial destructor
template <class FowardIterator>
inline void 
__destroy_aux(FowardIterator first, FowardIterator last, __false_type)
{
    for( ; first < last; ++first)
    {
        /* 好奇怪的语法... 把迭代器类型的东西转换成了对应的实际指针类型 */
        destroy(&*first);
    }
}

// 如果元素的数值型别 (value type) 有trivial destructor
template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_type) {}

// destroy() 第二版本针对迭代器为 char* (string类型？) 和 wchar_t* 的特化版本
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}