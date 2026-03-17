# vm.c Changes

## Overview
`vm.c` implements the `seginit` function, which initializes the Global Descriptor Table (GDT) for memory segmentation.

## Detailed Changes
- **seginit**:
  - Retrieves current CPU pointer using `cpus[cpuid()]`.
  - Sets up kernel and user segments:
    ```c
    c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
    c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
    c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, STARTPROC, (PROCSIZE << 12), DPL_USER);
    c->gdt[SEG_UDATA] = SEG(STA_W, STARTPROC, (PROCSIZE << 12), DPL_USER);
    ```
  - Calls `lgdt(c->gdt, sizeof(c->gdt));` to load the GDT.

## Impact
This code creates a flat memory model with separate privilege levels for kernel (0) and user (3). User segments are placed at the address defined by `STARTPROC` and limited by `PROCSIZE`. The routine is executed once per CPU during boot to configure segmentation.