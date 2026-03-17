# defs.h Changes

## Overview
This header added forward declarations for new process and context-switching routines and removed an unneeded time function.

## Detailed Changes
- Added `struct context;` forward declaration for use by `swtch`.
- Removed declaration of `cmostime`, since CMOS time retrieval isn't used anymore.
- Added process-related functions:
  - `void pinit(void);` – initialize the process table.
  - `void scheduler(void) __attribute__((noreturn));` – the scheduler entry point.
- Declared `void swtch(struct context*);` for the assembly context switcher.

## Impact
The updated declarations prepare other source files to call into the process-management subsystem and perform context switches. Unused declarations were cleaned up.