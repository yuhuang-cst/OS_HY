#练习1：给未被映射的地址映射上物理页
##【1.1】设计实现过程

设计与代码实现如下：

```
	//根据线性地址addr获得页表项，若页表不存在则创建页表
    ptep = get_pte(mm->pgdir, addr, 1);

    if(ptep == NULL){//获取ptep失败
        cprintf("get_pte in do_pgfault failed\n");
        goto failed;
    }

    if (*ptep == 0){ //若页表项对应的物理页帧不存在
        if (pgdir_alloc_page(mm->pgdir, addr, perm) == NULL){ //分配新的物理页帧
            cprintf("pgdir_alloc_page in do_pgfault failed\n");
            goto failed;
        }
    }
    else{ //页表项对应的物理页帧存在，则情况只能是物理页帧在硬盘中
        if (swap_init_ok) {
            struct Page *page = NULL;
            swap_in(mm, addr, &page);//选择内存中一页交换
            page_insert(mm->pgdir, page, addr, perm);//建立page与addr的映射
            swap_map_swappable(mm, addr, page, 1); //设置该页可交换
            page->pra_vaddr = addr; 
        }
        else {
            cprintf("no swap_init_ok but ptep is %x, failed\n",*ptep);
            goto failed;
        }
    }
    ret = 0;
failed:
    return ret;
```

##【1.2】请描述页目录项（Pag Director Entry）和页表（Page Table Entry）中组成部分对ucore实现页替换算法的潜在用处。
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
对ucore实现页替换算法的潜在用处：

```
*	可使用PTE_A实现简单时钟页替换算法
*	可使用PTE_D与PTE_A实现扩展时钟页替换算法
*	可使用PTE_A实现缺页率算法，在每次缺页时换出两次缺页时间中未访问到的页
```


##【1.3】如果ucore的缺页服务例程在执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？
*	保存现场，跳到缺页服务例程


#练习2：补充完成基于FIFO的页面替换算法
##【1.1】设计实现过程
设计与代码实现如下：

```
static int
_fifo_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in)
{
    list_entry_t *head=(list_entry_t*) mm->sm_priv;
    list_entry_t *entry=&(page->pra_page_link);
 
    assert(entry != NULL && head != NULL);
    //record the page access situlation
    /*LAB3 EXERCISE 2: 2013011363*/ 
    //(1)link the most recent arrival page at the back of the pra_list_head qeueue.

    list_add(head, entry);//将新置换进内存的页插入队尾
    return 0;
}

static int
_fifo_swap_out_victim(struct mm_struct *mm, struct Page ** ptr_page, int in_tick)
{
     list_entry_t *head=(list_entry_t*) mm->sm_priv;
         assert(head != NULL);
     assert(in_tick==0);
     /* Select the victim */
     /*LAB3 EXERCISE 2: 2013011363*/ 
     //(1)  unlink the  earliest arrival page in front of pra_list_head qeueue
     //(2)  set the addr of addr of this page to ptr_page

     list_entry_t *tail = head->prev; //取出队头，即内存中“最老”的页
     assert(tail != head); //若队列中没有可置换页，报错
     struct Page *page = le2page(tail, pra_page_link); //获得tail对应Page结构体
     assert(page != NULL);
     *ptr_page = page; //将选中的页置换出去
     list_del(tail); //删除被替换的页
     return 0;
}
```

##【1.2】现有的swap_manager框架是否足以支持在ucore中实现此算法？
*	支持，设计方案见chanllenge部分

###【1.2.1】需要被换出的页的特征是什么？

*	优先换出访问位为0，修改位也为0的页；其次是访问位为0，修改位为1的页；
*	具体见challenge部分的时钟算法

###【1.2.2】在ucore中如何判断具有这样特征的页？

*	设页表项为pte
*	(pte & PTE_A) == 0 && (pte & PTE_D) == 0：代表访问位为0，修改位为0
*	(pte & PTE_A) == 0 && (pte & PTE_D) != 0：代表访问位为0，修改位为1

###【1.2.3】何时进行换入和换出操作？

*	在发生页访问异常时执行换入换出操作

#Challenge
采用《操作系统：精髓与设计原理》（第七版）中给出的时钟算法：

```
设访问位为u，修改位为m：
*   未访问，未修改（u = 0; m = 0）
*   访问，未修改（u = 1; m = 0）
*   未访问，修改（u = 0; m = 1）
*   访问，修改(u = 1; m = 1)
算法如下：
1.  从指针的当前位置开始扫描，选择遇到的第一个（u = 0; m = 0）的页用于置换。
2.  若1失败，则第二遍扫描，选择遇到的第一个（u = 0; m = 1）的页用于置换。在扫描的过程中，将扫过的页的访问位u置0
3.  若2失败，则返回1
```
代码以及注释如下：

```
list_entry_t pra_list_head;

list_entry_t *clock_ptr;

/*
 * (2) _fifo_init_mm: init pra_list_head and let  mm->sm_priv point to the addr of pra_list_head.
 *              Now, From the memory control struct mm_struct, we can access FIFO PRA
 */
static int
_fifo_init_mm(struct mm_struct *mm)
{     
     list_init(&pra_list_head);
     mm->sm_priv = &pra_list_head;
     //cprintf(" mm->sm_priv %x in fifo_init_mm\n",mm->sm_priv);

     clock_ptr = (list_entry_t*) mm->sm_priv;
     return 0;
}
/*
 * (3)_fifo_map_swappable: According FIFO PRA, we should link the most recent arrival page at the back of pra_list_head qeueue
 */
static int
_fifo_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in)
{
    list_entry_t *head=(list_entry_t*) mm->sm_priv;
    list_entry_t *entry=&(page->pra_page_link);
 
    assert(entry != NULL && head != NULL);
    //record the page access situlation
    /*LAB3 EXERCISE 2: 2013011363*/ 
    //(1)link the most recent arrival page at the back of the pra_list_head qeueue.

    list_add(head, entry);//将新置换进内存的页插入队尾
    return 0;
}

static int
_fifo_swap_out_victim(struct mm_struct *mm, struct Page ** ptr_page, int in_tick)
{
     list_entry_t *head=(list_entry_t*) mm->sm_priv;
         assert(head != NULL);
     assert(in_tick==0);
     /* Select the victim */
     /*LAB3 EXERCISE 2: 2013011363*/ 
     //(1)  unlink the  earliest arrival page in front of pra_list_head qeueue
     //(2)  set the addr of addr of this page to ptr_page
     
     //clock
     clock_ptr = head;
     list_entry_t *old_clock_ptr = clock_ptr;
     while (1){
        bool find_0_0 = 0;
        do{
            if (clock_ptr != head){//若时钟指向head则跳过，因head并无实际意义
                //获得clock_ptr对应的Page
                struct Page *p = le2page(clock_ptr, pra_page_link);
                assert(p !=NULL);
                //获取对应页表项
                pte_t *ptep = get_pte(mm->pgdir , p->pra_vaddr , 1);
                //找到访问位为0，修改位为0的
                if( ((*ptep) & PTE_A) == 0 && ((*ptep) & PTE_D) == 0 ){
                    find_0_0 = 1;
                    list_del(clock_ptr);
                    clock_ptr = clock_ptr->next;
                    *ptr_page = p;
                    break;
                }
            }
            clock_ptr = clock_ptr->next;
        }while(clock_ptr != old_clock_ptr);

        if (find_0_0)
            break;

        bool find_0_1 = 0;
        do{
            if (clock_ptr != head){
                struct Page *p = le2page(clock_ptr, pra_page_link);
                assert(p !=NULL);
                //获取对应页表项
                pte_t *ptep = get_pte(mm->pgdir , p->pra_vaddr , 1);
                //找到访问位为0，修改位为1的页
                if( ((*ptep) & PTE_A) == 0 && ((*ptep) & PTE_D) != 0 ){
                    find_0_1 = 1;
                    list_del(clock_ptr);
                    clock_ptr = clock_ptr->next;
                    *ptr_page = p;
                    break;
                }
                //访问位设置为0
                *ptep = (*ptep) & (~PTE_A);
            }
            clock_ptr = clock_ptr->next;
        }while(clock_ptr != old_clock_ptr);

        if(find_0_1)
            break;
     }
```
以上代码存在bug，笔者表示实在调不出来。。。但是毕竟尝试做了，就交了吧。














