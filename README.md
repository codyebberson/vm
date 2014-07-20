vm
==

A simple VM impemented with computed dispatch in C. Build with `gcc vmtest.c vm.c -O2 -o vmtest --std=c99`. It's fast.
```
time (vmtest-switch-loop): 2.048s
time (vmtest-computed-goto): 0.798s
```
Based on [simple-virtual-machine](https://github.com/parrt/simple-virtual-machine).
