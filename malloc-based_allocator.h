#if 0
# include <new>
# define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)
# include <iostream>
# define __THROW_BAD_ALLOC cerr << "out of memory" << endl; exit(1)
#endif

// malloc-based allocator, 通常比稍后介绍的 default alloc 速度慢
// 一般而言是 thread-safe，并且对于空间的运用比较高效 (efficient)
// 以下是第一级配置器
// 注意，无 "template 型别参数"。至于 "非型别参数" inst，则完全没派上用场
template <int inst>
class __malloc_alloc_template {

private:
// 以下函数用来处理内存不足的情况
// oom : out of memory.
static void *oom_malloc(size_t);
static void *oom_realloc(void*, size_t);
// 函数指针，用于处理oom问题，即已经完全无法通过内存调整获取到能够进行分配到内存的时候，就调用这个函数进行处理
static void (* __malloc_alloc_oom_handler) ();

public:

static void * allocate(size_t n)
{
    void *result = malloc(n);               // 第一级配置器直接使用 malloc()
    // 以下无法满足需求时，改用 oom_malloc()
    if(0 == result) result = oom_malloc(n);
    return result;
}

static void deallocate(void* p, size_t /* n */)
{
    free(p);                                // 第一级配置器直接使用 free()
}

static void * reallocate(void *p, size_t /* old_sz */, size_t new_sz)
{
    void * result = realloc(p, new_sz);     // 第一级配置器直接使用 realloc()
    // 以下无法满足需求时，改用 oom_realloc()
    if (0 == result) result = oom_realloc(p, new_sz);
    return result;
}

// 以下仿真 C++ 的 set_new_handler()。换句话说，你可以通过它
// 指定你自己的 out-of-memory handler
static void (* set_malloc_handler(void (*f) ())) ()
{
    void (* old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return (old);
}
};

// malloc_alloc out-of-memory handling
// 初值为0，有待客端设定
template <int inst>
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler) () = 0;

template <int inst>
void * __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
    void (* my_malloc_handler) ();
    void *result;
    for (;;) {          // 不断尝试释放、配置、再释放、再配置...
        my_malloc_handler = __malloc_alloc_oom_handler;
        if(0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();         // 调用处理例程，企图释放内存
        result = malloc(n);             // 再次尝试配置内存
        if (result) return (result);
    }
}

template <int inst>
void * __malloc_alloc_template<inst>::oom_realloc(void* p, size_t n) 
{
    void (* my_malloc_handler) ();
    void *result;

    for (;;) {          // 不断尝试释放、配置、再释放、再配置...
        my_malloc_handler = __malloc_alloc_oom_handler;
        if(0 == my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*my_malloc_handler)();         // 调用处理例程，企图释放内存
        result = realloc(p, n);         // 再次尝试配置内存
        if (result) return (result);
    }
}

// 注意，以下直接将参数 inst 指定为 0
typedef __malloc_alloc_template<0> malloc_alloc;