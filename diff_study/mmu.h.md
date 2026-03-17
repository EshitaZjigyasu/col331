# mmu.h Changes

## Overview
`mmu.h` was revised to focus on segmentation rather than paging. Several new constants, structures, and macros support x86 segment descriptors.

## Detailed Changes
- Added `#define NSEGS 6` to define number of GDT entries.
- Introduced `struct segdesc` representing the 8-byte segment descriptor layout with bitfields for limit, base, type, dpl, etc.
- Added macro `SEG(type, base, lim, dpl)` to create a descriptor.
- Added attribute flags `STA_X`, `STA_W`, `STA_R` for executable, writable, readable segments.
- Removed or commented out legacy paging constants (`PGADDR`, `NPDENTRIES`, `PTE_P`, etc.).
- Removed `SEG_TSS` and `STS_T32A` definitions since TSS is not used here.

## Impact
These updates enable the kernel to set up its GDT entries with proper access rights. The segmentation infrastructure is now in place, and paging-related remnants are cleared, aligning the file with the current implementation.