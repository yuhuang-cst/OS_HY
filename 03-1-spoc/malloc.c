/*
连续内存管理：优先匹配算法
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MIN_SIZE 2048


union header {
	struct {
		union header *next;//下一个空闲内存块中header的位置
		unsigned int size;//header+存储区的大小（单位为sizeof(Header)）
	}content;
	long forAlign;
};

typedef union header Header;

Header *free_hp = NULL;

Header *my_free_intenal(Header *p){
	Header *prev, *next;
	//遍历空闲链表，找出prev、next，使prev < p < next
	for(prev = next = free_hp; next != NULL; prev = next, next = next->content.next){
		if(p < next)
			break;
	}
	prev->content.next = p;
	p->content.next = next;

	//合并空闲内存
	if(prev != NULL && (prev + prev->content.size == p)){
		prev->content.next = p->content.next;
		prev->content.size += p->content.size;
		p = prev;
	}
	if(next != NULL && (p + p->content.size == next)){
		p->content.next = next->content.next;
		p->content.size += next->content.size;
	}
	return p;
}

//@param nunit: 需要分配的单元数目
//@return： 新分配的内存的头（经过合并）
//分配nuint * sizeof(Header)字节的内存
Header *my_morecore(unsigned int nunit)
{
	void *p;
	Header *hp;
	Header *free_curp;
	//nunit *= 2;//多分配一倍，避免分配次数过多
	//if(nunit < MIN_SIZE)
	//	nunit = MIN_SIZE;

	p = sbrk(nunit * sizeof(Header));

	if (p == (void*)-1){
		//分配失败
		return NULL;
	}
	hp = (Header *)p;
	hp->content.size = nunit;

	//释放新开的内存
	free_curp = my_free_intenal(hp);
	return free_curp;
}

void *my_malloc(unsigned int nbytes)
{
	//计算需要多少个nunit。nbytes == 0 与 nbytes % sizeof(Header) == 0的情况均已考虑进去
	unsigned int nunit = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
	Header *p;
	Header *prev = NULL;

	if(free_hp == NULL){
		free_hp = (Header *)sbrk(sizeof(Header));
		free_hp->content.next = NULL;
		free_hp->content.size = 1;
	}
	//最先匹配，寻找可用空闲内存
	for(p = free_hp; ; prev = p, p = p->content.next){
		if (p == NULL){//没有合适的空闲内存
			if (my_morecore(nunit) == NULL) return NULL;
			p = free_hp;
			prev = NULL;
		}
		if (p->content.size >= nunit){
			if (p->content.size == nunit){
				if (prev != NULL)
					prev->content.next = p->content.next;
			}
			else{//p->content.size > nunit
				p->content.size -= nunit;
				p += p->content.size;
				p->content.size = nunit;
			}
			return (void *)(p + 1);
		}
	}
}

void my_free(void *p){
	my_free_intenal( ((Header *)p) - 1);
}


int main()
{
	void *p1 = my_malloc(16);
	void *p2 = my_malloc(16);
	void *p3 = my_malloc(16);
	printf("p1 = %p, p2 = %p, p3 = %p\n", p1, p2, p3);

	printf("begin:\n");
	for (Header *p = free_hp; p != NULL; p = p->content.next){
		printf("%p %d\n", p, p->content.size);
	}

	my_free(p2);
	printf("after my_free(p2):\n");
	for (Header *p = free_hp; p != NULL; p = p->content.next){
		printf("%p %d\n", p, p->content.size);
	}

	my_free(p3);
	printf("after my_free(p3):\n");
	for (Header *p = free_hp; p != NULL; p = p->content.next){
		printf("%p %d\n", p, p->content.size);
	}

	my_free(p1);
	printf("after my_free(p1):\n");
	for (Header *p = free_hp; p != NULL; p = p->content.next){
		printf("%p %d\n", p, p->content.size);
	}
	
	return 0;
}

