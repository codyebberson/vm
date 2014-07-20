
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
// N = 10                      ADDRESS
    LOAD, -3,
    ICONST, 2,
    ILT,
    BRF, 10,
    ICONST, 1,
    RET,

    LOAD, -3,
    LOAD, -3,
    ICONST, 1,
    ISUB,
    CALL, 0, 1,
    IMUL,
    RET,

    ICONST, 1000000, //23
    GSTORE, 0,  //25
    ICONST, 0,  //27
    GSTORE, 1,  //29

    GLOAD, 1,   //31
    GLOAD, 0,   //33
    ILT,        //34
    BRF, 54,    //35
    GLOAD, 1,   //37
    ICONST, 1,  //39
    IADD,       //40
    GSTORE, 1,  //42

    //call fact
    ICONST, 12, //44
    CALL, 0, 1, //46
    POP, // 49

    BR, 30, //50
    GLOAD, 1, //52

    HALT //54
};

int main(int argc, char *argv[])
{
    int t1 = 0;
    int t2 = 0;
    vm_exec(hello, sizeof(hello), 0, 0, 1);
    t1 = (clock() / (CLOCKS_PER_SEC / 1000));
    vm_exec(loop, sizeof(loop), 22, 2, 1);
    t2 = (clock() / (CLOCKS_PER_SEC / 1000));
    printf("duration = %d ms\n", (t2 - t1));
    return 0;
}

