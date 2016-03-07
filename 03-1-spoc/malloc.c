#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

union Header
{
	struct {
		union Header *next;
		unsigned int size;
	}s;
	long forAlign;
}

Header *morecore()
{
	
}

void *malloc(unsigned int size)
{

}

void free(void *p)
{

}

int main()
{
	sbrk(8);

	return 0;
}