#include  <stdio.h>
#include <ulib.h>

int main(void)
{
    int CS = 0;
    asm volatile(
        "movl %%cs, %0\n"
        : "=r"(CS)
    	);
    cprintf("user mode's CPL = %d\n", CS & 3);
}