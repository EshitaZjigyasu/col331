# memlayout.h (New File)

## Overview
Defines physical memory layout constants for the kernel and user processes.

## Definitions
- `STARTPROC` – 0x200000: physical address where user processes begin (2 MB).
- `PROCSIZE` – 0x100000: size of each user process region (1 MB).

## Impact
These constants are used when setting up user segments in `vm.c` and to manage process memory allocation boundaries. They formalize the simple fixed-region memory model used by xv6.