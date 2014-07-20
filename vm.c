#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

#define DEFAULT_STACK_SIZE 1024
#define FALSE 0
#define TRUE 1

typedef struct {
    char name[6];
    int nargs;
} VM_INSTRUCTION;

VM_INSTRUCTION vm_instructions[] = {
    { "noop",   0 },
    { "iadd",   0 },
    { "isub",   0 },
    { "imul",   0 },
    { "ilt",    0 },
    { "ieq",    0 },
    { "ret",    0 },
    { "br",     1 },
    { "brt",    1 },
    { "brf",    1 },
    { "iconst", 1 },
    { "load",   1 },
    { "gload",  1 },
    { "store",  1 },
    { "gstore", 1 },
    { "print",  0 },
    { "pop",    0 },
    { "halt",   0 },
    { "call",   2 },
    { "ret",    0 }
};

static void vm_print_instr(int *code, int ip);
static void vm_print_stack(int *stack, int count);
static void vm_print_data(int *globals, int count);

void vm_exec(int *code, int count, int startip, int nglobals, int trace)
{
    static void* dispatch_table[19] = { NULL, &&do_IADD, &&do_ISUB, &&do_IMUL, &&do_ILT, &&do_IEQ, &&do_BR, &&do_BRT, &&do_BRF,
    &&do_ICONST, &&do_LOAD, &&do_GLOAD, &&do_STORE, &&do_GSTORE, &&do_PRINT, &&do_POP,
    &&do_HALT, &&do_CALL, &&do_RET };
    #define DISPATCH() goto *dispatch_table[code[ip++]]
    // registers
    int ip = startip ;         // instruction pointer register
    int sp = -1;          // stack pointer register
    int fp = -1;        // frame pointer register

    int opcode = code[ip];
    int a = 0;
    int b = 0;
    int addr = 0;
    int offset = 0;
    int rvalue = 0;
    int nargs  = 0;

    // global variable space
    int globals[nglobals];

    // Operand stack, grows upwards
    int stack[DEFAULT_STACK_SIZE];
    DISPATCH();
           do_IADD:
               b = stack[sp--];           // 2nd opnd at top of stack
               a = stack[sp--];           // 1st opnd 1 below top
               stack[++sp] = a + b;       // push result
               DISPATCH();
           do_ISUB:
               b = stack[sp--];
               a = stack[sp--];
               stack[++sp] = a - b;
               DISPATCH();
           do_IMUL:
               b = stack[sp--];
               a = stack[sp--];
               stack[++sp] = a * b;
               DISPATCH();
           do_ILT:
               b = stack[sp--];
               a = stack[sp--];
               stack[++sp] = (a < b) ? TRUE : FALSE;
               DISPATCH();
           do_IEQ:
               b = stack[sp--];
               a = stack[sp--];
               stack[++sp] = (a == b) ? TRUE : FALSE;
               DISPATCH();
           do_BR:
               ip = code[ip];
               DISPATCH();
           do_BRT:
               addr = code[ip++];
               if (stack[sp--] == TRUE) ip = addr;
               DISPATCH();
           do_BRF:
               addr = code[ip++];
               if (stack[sp--] == FALSE) ip = addr;
               DISPATCH();
           do_ICONST:
               stack[++sp] = code[ip++];  // push operand
               DISPATCH();
           do_LOAD: // load local or arg; 1st local is fp+1, args are fp-3, fp-4, fp-5, ...
               offset = code[ip++];
               stack[++sp] = stack[fp+offset];
               DISPATCH();
           do_GLOAD: // load from global memory
               addr = code[ip++];
               stack[++sp] = globals[addr];
               DISPATCH();
           do_STORE:
               offset = code[ip++];
               stack[fp+offset] = stack[sp--];
               DISPATCH();
           do_GSTORE:
               addr = code[ip++];
               globals[addr] = stack[sp--];
               DISPATCH();
           do_PRINT:
               printf("%d\n", stack[sp--]);
               DISPATCH();
           do_POP:
               --sp;
               DISPATCH();
           do_CALL:
               addr = code[ip++];          // target addr of function
               nargs = code[ip++];     // how many args got pushed
               stack[++sp] = nargs;        // save num args
               stack[++sp] = fp;           // save fp
               stack[++sp] = ip;           // push return address
               fp = sp;                    // fp points at ret addr on stack
               ip = addr;
               DISPATCH();
           do_RET:
               rvalue = stack[sp--];   // pop return value
               sp = fp;                    // jump over locals to fp which points at ret addr
               ip = stack[sp--];           // pop return address, jump to it
               fp = stack[sp--];           // restore fp
               nargs = stack[sp--];        // how many args to throw away?
               sp -= nargs;                // pop args
               stack[++sp] = rvalue;       // leave result on stack
               DISPATCH();
           do_HALT:
               ip = count;
}

static void vm_print_instr(int *code, int ip)
{
    int opcode = code[ip];
    VM_INSTRUCTION *inst = &vm_instructions[opcode];
    switch (inst->nargs) {
    case 0:
        printf("%04d:  %-20s", ip, inst->name);
        break;

    case 1:
        printf("%04d:  %-10s%-10d", ip, inst->name, code[ip + 1]);
        break;
    }
}

static void vm_print_stack(int *stack, int count)
{
    printf("stack=[");
    for (int i = 0; i <= count; i++) {
        printf(" %d", stack[i]);
    }
    printf(" ]\n");
}

static void vm_print_data(int *globals, int count)
{
    printf("Data memory:\n");
    for (int i = 0; i < count; i++) {
        printf("%04d: %d\n", i, globals[i]);
    }
}

