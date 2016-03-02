#include  <stdio.h>
#include <ulib.h>

int main(void)
{
    int CPL = 0;
    asm volatile(
        "movl %%cs, %0\n"
        : "=r"(CPL)
    	);
    cprintf("user mode's CPL = %d\n", CPL & 3);
}