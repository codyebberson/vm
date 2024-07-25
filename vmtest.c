
#include <stdio.h>
#include <time.h>
#include "vm.h"

int hello[] = {
    ICONST, 1234,
    PRINT,
    HALT
};

int loop[] = {
// .GLOBALS 2; N, I
// N = 200000000               ADDRESS
        ICONST, 200000000,     // 0
        GSTORE, 0,             // 2
// I = 0
        ICONST, 0,             // 4
        GSTORE, 1,             // 6
// WHILE I<N:
// START (8):
        GLOAD, 1,              // 8
        GLOAD, 0,              // 10
        ILT,                   // 12
        BRF, 24,               // 13
//     I = I + 1
        GLOAD, 1,              // 15
        ICONST, 1,             // 17
        IADD,                  // 19
        GSTORE, 1,             // 20
        BR, 8,                 // 22
// DONE (24):
// PRINT "LOOPED "+N+" TIMES."
        HALT                   // 24
};

int main(int argc, char *argv[])
{
    clock_t start;
    clock_t end;
    double duration;

    // Run the "hello" program
    vm_exec(hello, sizeof(hello), 0, 0, 0);

    // Run the "loop" program
    start = clock();
    vm_exec(loop, sizeof(loop), 0, 2, 0);
    end = clock();
    duration = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("loop duration = %f sec\n", duration);

    return 0;
}

