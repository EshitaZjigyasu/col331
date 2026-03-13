# main.c Changes

## Overview
The changes in `main.c` include adding new header includes and calling `seginit()` to initialize segment descriptors before the welcome message.

## Detailed Changes
- **Includes**: Added `#include "mmu.h"`, `#include "proc.h"`, and `#include "memlayout.h"` to support memory management and process-related structures.
- **main function**: Added `seginit(); // segment descriptors` call before `welcome();`. This initializes the CPU's segment descriptors for memory segmentation.

## Impact
These changes prepare the system for process management by setting up memory segments. The `seginit` call ensures that the GDT is properly configured with kernel and user segments before proceeding to user interaction.