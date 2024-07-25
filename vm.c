#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

#define MAX_GLOBALS 1024
#define STACK_SIZE 1024
#define FALSE 0
#define TRUE 1

#define SAFE_MODE 1

typedef struct {
    char name[8];
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
    { "halt",   0 }
};

static void vm_print_instr(int *code, int ip);
static void vm_print_stack(int *stack, int count);
static void vm_print_data(int *globals, int count);
static void vm_panic(const char* format, ...);

void vm_exec(int *code, int count, int startip, int nglobals, int trace)
{
    // registers
    int ip = 0;         // instruction pointer register
    int sp = -1;        // stack pointer register
    int fp = -1;        // frame pointer register

    int opcode = code[ip];
    int a = 0;
    int b = 0;
    int addr = 0;
    int offset = 0;

    // global variable space
    int globals[MAX_GLOBALS];

    // Operand stack, grows upwards
    int stack[STACK_SIZE];

    while (opcode != HALT && ip < count) {
        if (trace) vm_print_instr(code, ip);
        ip++; //jump to next instruction or to operand
        switch (opcode) {
            case IADD:
#ifdef SAFE_MODE
                if (sp < 1) {
                  vm_panic("IADD stack underflow: %d\n", sp);
                }
#endif
                b = stack[sp--];           // 2nd opnd at top of stack
                a = stack[sp--];           // 1st opnd 1 below top
                stack[++sp] = a + b;       // push result
                break;
            case ISUB:
#ifdef SAFE_MODE
                if (sp < 1) {
                  vm_panic("ISUB stack underflow: %d\n", sp);
                }
#endif
                b = stack[sp--];
                a = stack[sp--];
                stack[++sp] = a - b;
                break;
            case IMUL:
#ifdef SAFE_MODE
                if (sp < 1) {
                  vm_panic("IMUL stack underflow: %d\n", sp);
                }
#endif
                b = stack[sp--];
                a = stack[sp--];
                stack[++sp] = a * b;
                break;
            case ILT:
#ifdef SAFE_MODE
                if (sp < 1) {
                  vm_panic("ILT stack underflow: %d\n", sp);
                }
#endif
                b = stack[sp--];
                a = stack[sp--];
                stack[++sp] = (a < b) ? TRUE : FALSE;
                break;
            case IEQ:
#ifdef SAFE_MODE
                if (sp < 1) {
                  vm_panic("IEQ stack underflow: %d\n", sp);
                }
#endif
                b = stack[sp--];
                a = stack[sp--];
                stack[++sp] = (a == b) ? TRUE : FALSE;
                break;
            case BR:
#ifdef SAFE_MODE
                if (ip >= count) {
                  vm_panic("BR target out of range: %d\n", ip);
                }
#endif
                ip = code[ip];
                break;
            case BRT:
#ifdef SAFE_MODE
                if (ip + 1 >= count) {
                  vm_panic("BRT target out of range: %d\n", ip);
                }
                if (sp < 0) {
                  vm_panic("BRT stack underflow: %d\n", sp);
                }
#endif
                addr = code[ip++];
                if (stack[sp--] == TRUE) ip = addr;
                break;
            case BRF:
#ifdef SAFE_MODE
                if (ip + 1 >= count) {
                  vm_panic("BRF target out of range: %d\n", ip);
                }
                if (sp < 0) {
                  vm_panic("BRF stack underflow: %d\n", sp);
                }
#endif
                addr = code[ip++];
                if (stack[sp--] == FALSE) ip = addr;
                break;
            case ICONST:
#ifdef SAFE_MODE
                if (ip + 1 >= count) {
                  vm_panic("ICONST target out of range: %d\n", ip);
                }
                if (sp + 1 >= STACK_SIZE) {
                  vm_panic("ICONST stack overflow: %d\n", sp);
                }
#endif
                stack[++sp] = code[ip++];  // push operand
                break;
            case LOAD: // load local or arg; 1st local is fp+1, args are fp-3, fp-4, fp-5, ...
#ifdef SAFE_MODE
                if (ip + 1 >= count) {
                  vm_panic("LOAD target out of range: %d\n", ip);
                }
                if (fp + offset < 0 || fp + offset >= STACK_SIZE) {
                  vm_panic("LOAD stack out of range: %d\n", fp + offset);
                }
                if (sp + 1 >= STACK_SIZE) {
                  vm_panic("LOAD destination stack overflow: %d\n", sp);
                }
#endif
                offset = code[ip++];
                stack[++sp] = stack[fp+offset];
                break;
            case GLOAD: // load from global memory
#ifdef SAFE_MODE
                if (ip + 1 >= count) {
                  vm_panic("GLOAD target out of range: %d\n", ip);
                }
                if (sp + 1 >= STACK_SIZE) {
                  vm_panic("GLOAD stack overflow: %d\n", sp);
                }
#endif
                addr = code[ip++];
                stack[++sp] = globals[addr];
                break;
            case STORE:
#ifdef SAFE_MODE
                if (ip + 1 >= count) {
                  vm_panic("STORE target out of range: %d\n", ip);
                }
                if (fp + offset < 0 || fp + offset >= STACK_SIZE) {
                  vm_panic("STORE stack out of range: %d\n", fp + offset);
                }
#endif
                offset = code[ip++];
                stack[fp+offset] = stack[sp--];
                break;
            case GSTORE:
#ifdef SAFE_MODE
                if (ip + 1 >= count) {
                  vm_panic("GSTORE target out of range: %d\n", ip);
                }
                if (sp < 0) {
                  vm_panic("GSTORE stack underflow: %d\n", sp);
                }
#endif
                addr = code[ip++];
                globals[addr] = stack[sp--];
                break;
            case PRINT:
#ifdef SAFE_MODE
                if (sp < 0) {
                  vm_panic("PRINT stack underflow: %d\n", sp);
                }
#endif
                printf("%d\n", stack[sp--]);
                break;
            case POP:
#ifdef SAFE_MODE
                if (sp < 1) {
                  vm_panic("POP stack underflow: %d\n", sp);
                }
#endif
                --sp;
                break;
            default:
                vm_panic("invalid opcode: %d at ip=%d\n", opcode, (ip - 1));
        }
        if (trace) vm_print_stack(stack, sp);
        opcode = code[ip];
    }
    if (trace) vm_print_instr(code, ip);
    if (trace) vm_print_stack(stack, sp);
    if (trace) vm_print_data(globals, nglobals);
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

static void vm_panic(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}
