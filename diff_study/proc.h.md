# proc.h Changes

## Overview
The `proc.h` file was updated to include the GDT in the CPU structure.

## Detailed Changes
- **struct cpu**: Added `struct segdesc gdt[NSEGS]; // x86 global descriptor table` to store per-CPU segment descriptors.

## Impact
This allows each CPU to have its own GDT, enabling per-CPU segment initialization in `seginit()`. This is essential for memory segmentation in a multi-CPU environment, though currently simplified to single CPU.