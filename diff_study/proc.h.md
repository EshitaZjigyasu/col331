# proc.h Changes

## Overview
Added GDT storage to the CPU structure.

## Detailed Changes
- In `struct cpu`, inserted:
  ```c
  struct segdesc gdt[NSEGS]; // x86 global descriptor table
  ```

## Impact
This allows each CPU to maintain its own set of segment descriptors so that `seginit` can initialize the GDT per processor. Necessary for segmentation support and future multiprocessor correctness.