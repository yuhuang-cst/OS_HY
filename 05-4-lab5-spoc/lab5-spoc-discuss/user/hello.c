#include <stdio.h>
#include <ulib.h>

int
main(void) {
    cprintf("Hello world!!.\n");

    cprintf("I am process %d. ", getpid());
    //print_pgdir();


    int CS = 0;
    asm volatile(
        "movl %%cs, %0\n"
        : "=r"(CS)
    	);
    cprintf("CPL = %d\n", CS & 3);


    cprintf("hello pass.\n");
    return 0;
}

