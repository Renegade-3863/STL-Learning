enum {__ALIGN = 8};         // 小型区块的上调边界，小内存块都是这个对齐值的整数倍，整个内存池维护一些对应内存的链表
enum {__MAX_BYTES = 128};   // 小型区块的上限，大于这个值的内存申请就不再使用二级分配器分配内存了
enum {__NFREELISTS = __MAX_BYTES/__ALIGN};      // free_lists 个数

// 以下是第二级配置器
// 注意，无 "template 型别参数"，且第二参数完全没派上用场
// 第一参数用于多线程环境下，本书不讨论多线程环境
template <bool threads, int inst>
class __default_alloc_template
{

private:
    // ROUND_UP() 将 bytes 上调至 8 的倍数
    static size_t ROUND_UP(size_t bytes) {
        return (((bytes) + __ALIGN-1) & ~(__ALIGN - 1));
    }
private:
    union obj {                 // free-lists的节点构造
        union obj * free_list_link;
        char client_data[1];            /* The client sees this. */
    };
private:
    /// 16个free-lists
    static obj * volatile free_list[__NFREELISTS];
    // 以下函数根据区块大小，决定使用第 n 号 free-list。n 从 0 起算
    static size_t FREELIST_INDEX(size_t bytes) {
        return (((bytes) + __ALIGN-1)/__ALIGN - 1);
    }
};