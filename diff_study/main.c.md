# main.c Changes

## Overview
`main.c` gained new includes for memory and process support and now initializes segment descriptors.

## Detailed Changes
- Added includes:
  ```c
  #include "mmu.h"
  #include "proc.h"
  #include "memlayout.h"
  ```
- In `main()`, before calling `welcome()`, the call `seginit(); // segment descriptors` was inserted.

## Impact
These additions ensure that the kernel sets up its memory segmentation (GDT) before interacting with user-level code. The new headers provide definitions needed by segmentation and process code.
