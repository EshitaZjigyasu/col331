# Makefile Changes

## Overview
The Makefile was updated to include `vm.o` in the `OBJS` list, ensuring that the new `vm.c` file is compiled and linked into the kernel.

## Detailed Changes
- **Addition to OBJS**: Added `vm.o` to the list of object files. This is necessary because `vm.c` contains the `seginit` function, which is called in `main.c` to initialize segment descriptors for memory management.

## Impact
Without this change, the build would fail due to undefined references to functions in `vm.c`. This integrates virtual memory initialization into the kernel build process.