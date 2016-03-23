####请参考lab3_result的代码，思考如何在lab3_results中实现clock算法，给出你的概要设计方案。可4人一个小组。要求说明你的方案中clock算法与LRU算法上相比，潜在的性能差异性。进而说明在lab3的LRU算法实现的可能性评价（给出理由）。

####【1】ucore中clock算法设计
1.	采用《操作系统：精髓与设计原理》（第七版）中给出的时钟算法：

```
设访问位为u，修改位为m：
*	未访问，未修改（u = 0; m = 0）
*	访问，未修改（u = 1; m = 0）
*	未访问，修改（u = 0; m = 1）
*	访问，修改(u = 1; m = 1)
算法如下：
1.	从指针的当前位置开始扫描，选择遇到的第一个（u = 0; m = 0）的页用于置换。
2.	若1失败，则第二遍扫描，选择遇到的第一个（u = 0; m = 1）的页用于置换。在扫描的过程中，将扫过的页的访问位u置0
3.	若2失败，则返回1
```
根据上述大体思路，具体到ucore中的代码实现思路如下：

```
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

static int
_fifo_swap_out_victim(struct mm_struct *mm, struct Page ** ptr_page, int in_tick)
{
     list_entry_t *head=(list_entry_t*) mm->sm_priv;
         assert(head != NULL);
     assert(in_tick==0);
     /* Select the victim */
     /*LAB3 EXERCISE 2: YOUR CODE*/ 
     //(1)  unlink the  earliest arrival page in front of pra_list_head qeueue
     //(2)  set the addr of addr of this page to ptr_page
     /* Select the tail */

     /*
     list_entry_t *le = head->prev;
     assert(head!=le);
     struct Page *p = le2page(le, pra_page_link);
     list_del(le);
     assert(p !=NULL);
     *ptr_page = p;
     return 0;*/

     //clock
     list_entry_t *old_clock_ptr = clock_ptr;
     while (1){
        bool find_0_0 = 0;
        do{
            if (clock_ptr != head){//若时钟指向head则跳过，因head并无实际意义
                //获得clock_ptr对应的Page
                struct Page *p = le2page(clock_ptr, pra_page_link);
                assert(p !=NULL);
                //获取对应页表项
                pte_t *ptep = get_pte(mm->pgdir , p->pra_vaddr , 0);
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
                pte_t *ptep = get_pte(mm->pgdir , p->pra_vaddr , 0);
                //找到访问位为0，修改位为1的页
                if( ((*ptep) & PTE_A) == 0 && ((*ptep) & PTE_D) == 1 ){
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

     return 0;

}
```
修改了_fifo_init_mm与_fifo_swap_out_victim两个函数。
以上代码由于未知的原因还不能work，由于8：00前要上交，故只能先交上一个有bug的版本，但思路已经能看的很清楚。

####【2】clock算法与LRU算法上相比，潜在的性能差异性。
LRU算法需要维护一个按最近访问时间维护的链表或栈，每一次的访问都需要调整该链表的顺序，故性能较低。
由于LRU维护的信息多于clock算法，LRU算法的缺页率将低于clock算法


####【3】进而说明在lab3的LRU算法实现的可能性评价（给出理由）
无法实现，需要维护一个按最近访问时间的Page链表，每次访问都需要修改这个链表的顺序，这件事只能由硬件来完成，而硬件并没有该功能。
