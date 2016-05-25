#2013011363 黄予 计34
#练习1：实现 first-fit 连续物理内存分配算法
##设计实现过程
设计的实现思路：

1.	实际上我们需要管理一个Page数组，数组中的每一项与实际内存中的每一4KB的页有一一对应的关系，现在来说明如何使用first-fit算法来管理这个Page数组，从而管理实际的内存页。下述“空闲内存区域”是针对Page数组而言的
2.	每个空闲内存区域的大小均为struct Page大小的整数倍，大小为n * sizeof(Page)的空闲区域都分为两部分，第一部分为struct Page头，剩下的部分大小为(n - 1) * sizeof(Page)。
3.	维护一个双向链表，这个链表将所有空闲区域的Page头串起来
4.	当需要分配内存时，从双向链表的头部开始寻找合适的空闲区域，若找到的空闲区域的空间大于所需分配的空间，则还需将找到的空闲区域分割，将剩下的空闲部分串回双向链表中。
5.	当需要释放内存时，将所释放区域的前后空闲内存与所释放的区域合并后，按地址插入双向链表中。

源代码以及注释如下：

```
free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

static void
default_init(void) {
    //初始化free_list双向循环链表，即free_list的next与prev均为自身
    list_init(&free_list);
    nr_free = 0;
}

static void
default_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        //检查此页是否为保留页
        assert(PageReserved(p));
        //flag = 0代表该页非保留，property项不起作用；property = 0代表free block大小为0
        p->flags = p->property = 0;
        //设置该页的引用计数为0
        set_page_ref(p, 0);
    }
    //分配n页
    base->property = n;
    //将base->flags的PG_property位置1，即base->property起作用
    SetPageProperty(base);
    //当前有空闲内存n
    nr_free += n;
    //将初始的空闲内存块加入空闲链表
    list_add(&free_list, &(base->page_link));
}

static struct Page *
default_alloc_pages(size_t n) {
    assert(n > 0);
    //检查总空间是否不够
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    //记录找到的空闲链表的前一项，以便按照地址进行插入
    list_entry_t *last_le = &free_list;
    list_entry_t *le = &free_list;
    //遍历空闲链表，寻找链表中第一个能匹配的内存块
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        //找到匹配的内存块
        if (p->property >= n) {
            last_le = list_prev(le);
            page = p;
            break;
        }
    }
    //若找到匹配的内存块
    if (page != NULL) {
        list_del(&(page->page_link));
        //若匹配的内存块的空间大于所需，则分割该空间
        if (page->property > n) {
            struct Page *p = page + n;
            p->property = page->property - n;
            //改进：按地址插入
            list_add(last_le, &(p->page_link));
            //list_add(&free_list, &(p->page_link));
        }
        nr_free -= n;
        //将page->flags的PG_property置0，代表page->property失效
        ClearPageProperty(page);
    }
    return page;
}

static void
default_free_pages(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    //重置标志位，在连续内存管理中，Page->ref应该不会用到吧
    for (; p != base + n; p ++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    //使得base->property有效
    SetPageProperty(base);
    list_entry_t *le = list_next(&free_list);
    //重新遍历链表，合并base前后的空闲内存块
    while (le != &free_list) {
        p = le2page(le, page_link);
        le = list_next(le);
        //合并紧邻base之后的空闲内存
        if (base + base->property == p) {
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
        }
        //合并紧邻base之前的空闲内存
        else if (p + p->property == base) {
            p->property += base->property;
            ClearPageProperty(base);
            base = p;
            list_del(&(p->page_link));
        }
    }
    nr_free += n;
    //将释放的内存插入按地址排序的空闲链表
    le = list_next(&free_list);
    while(le < &(base->page_link) && le != &free_list)
        le = list_next(le);
    list_add_before(le, &(base->page_link));

    //list_add(&free_list, &(base->page_link));
}
```
原实现中存在的问题是，双向链表并未按地址进行排序，笔者对此做了一些改进。除此之外，实际上仍有改进的空间，例如在分割内存时，可将分配的内存放在空闲内存块的后一部分，只需将原空闲内存快的Page头中的property减去分配出去的大小，而无须改变双向链表。

#练习2：实现寻找虚拟地址对应的页表项
####设计实现过程：
二级页表的地址为页目录项的高20位左移12位，若二级页表不存在则创建；以虚拟地址的中间10位为索引值，找到二级页表的页表项，返回

```
pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
    pde_t *pdep = &pgdir[PDX(la)];//取出页目录项
    if (*pdep & PTE_P == 0){//若不存在对应的二级页表，则根据情况创建二级页表
        struct Page *new_page = NULL;
        if (create != 0 && (new_page = alloc_page()) != NULL){//允许创建二级页表并且创建成功
            set_page_ref(new_page, 1);//设置该二级页表有一个引用
            uintptr_t phy_addr = page2pa(new_page);//取出二级页表的起始物理地址
            memset(KADDR(phy_addr), 0, PGSIZE);//将该页所有位置0
            *pdep = phy_addr | PTE_P | PTE_W | PTE_U;//设置页目录项的标志位
        }
        else
            return NULL;
    }
    return &((pte_t *)KADDR(PDE_ADDR(*pdep)))[PTX(la)];//返回pdep对应的二级页表地址加上la中的偏移，即二级页表项的地址
}
```

####【2.1】请描述页目录项（Pag Director Entry）和页表（Page Table Entry）中每个组成部分的含义和以及对ucore而言的潜在用处。

mmu.h中的源码：

```
/* page table/directory entry flags */
#define PTE_P           0x001                   // Present
#define PTE_W           0x002                   // Writeable
#define PTE_U           0x004                   // User
#define PTE_PWT         0x008                   // Write-Through
#define PTE_PCD         0x010                   // Cache-Disable
#define PTE_A           0x020                   // Accessed
#define PTE_D           0x040                   // Dirty
#define PTE_PS          0x080                   // Page Size
#define PTE_MBZ         0x180                   // Bits must be zero
#define PTE_AVAIL       0xE00                   // Available for software use
                                                // The PTE_AVAIL bits aren't used by the kernel or interpreted by the
                                                // hardware, so user processes are allowed to set them arbitrarily.
```
页目录项：

```
*	前20位：页表的高20位地址，右移12位即为其所指的页表所在的地址
*	后12位：一些标志位，从第0位到第7位依次为：PTE_P PTE_W PTE_U PTE_PWT PTE_PCD PTE_A PTE_D PTE_PS，第7位与第8位组合为PTE_MBZ，第9、10、11位组合为PTE_AVAIL
```
页表项：

```
*	前20位：物理页帧的高20位地址，右移12位即为其所指的物理页帧所在的地址
*	后12位：同页目录项
```
对ucore的好处：

```
ucore可使用标志位设置一些权限，例如是否可写，是否可被用户态访问；可使用这些标记记录一些改变，例如该页是否是脏页，若是脏页，则该页从内存中换出时，应重新写到硬盘中，否则可以不用这一步。
```


####【2.2】如果ucore执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？
若是缺页引起的异常，则将相应页从硬盘中调入内存，从异常返回

#练习3：释放某虚地址所在的页并取消对应二级页表项的映射

####设计实现过程：
源代码以及注释如下：

```
static inline void
page_remove_pte(pde_t *pgdir, uintptr_t la, pte_t *ptep) {
    if (*ptep & PTE_P){//检查pte是否valid
        struct Page *page = pte2page(*ptep);//获取pte指向的物理页帧
        if(page_ref_dec(page) == 0)//减少对需要释放物理页帧的引用，若引用为0，释放物理页帧
            free_page(page);
        *ptep = 0;//清空页表项
        tlb_invalidate(pgdir, la);//TLB的相应项设置标志位
    }
}
```
####【3.1】数据结构Page的全局变量（其实是一个数组）的每一项与页表中的页目录项和页表项有无对应关系？如果有，其对应关系是啥？

*	有，Page数组——pages的每一项对应实际可用物理内存中的每一4KB的页，通过page2pa()函数即可完成二者的转换。页目录项或页表项的高20位作为index,pages[index]即可取出页目录项或页表项指向的物理页帧对应的Page数组中的元素。
*	实际上ucore是这么管理内存的：以页为单位分配内存，实际分配内存与释放内存使用的是first-fit连续内存管理，再此基础上加入虚拟地址机制，即建立一个二级页表，从而完成虚拟地址到物理地址的转换。

####【3.2】如果希望虚拟地址与物理地址相等，则需要如何修改lab2，完成此事？ 
将KERNBASE改为0














