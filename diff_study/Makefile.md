# Makefile Changes

## Overview
The kernel build system was extended to support context switching and initial user code. Several new object files and build targets were added.

## Detailed Changes
- **Objects**:
  - Added `swtch.o` to `OBJS` so that the context-switching code in `swtch.S` is compiled and linked.
  - `vm.o` was already present but is now used with the new features.

- **New Target (`initcode`)**:
  - Compiles `initcode.S` with special flags (`-nostdinc`, `-I.`, etc.).
  - Links it as a binary blob (`initcode.out`) at text address 0.
  - Produces `initcode` (binary) and `initcode.asm` for inspection.

- **Kernel Target Update**:
  - Added dependency on `initcode`.
  - Linked kernel with `-b binary initcode` to embed the initial user program.

## Impact
This modification lays groundwork for multiprocessing support by including context switch code. Embedding `initcode` enables the kernel to start a first user process. The new build steps are essential when adding process management code.