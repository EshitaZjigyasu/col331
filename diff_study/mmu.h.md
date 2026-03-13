# mmu.h Changes

## Overview
The `mmu.h` file was significantly updated to include segment descriptor structures and macros for memory segmentation, while removing some unused definitions.

## Detailed Changes
- **SEG_TSS**: Removed the definition for task state segment.
- **NSEGS**: Added definition as 6, representing the number of segments.
- **struct segdesc**: Added a new structure defining the segment descriptor with bit fields for limit, base, type, privilege level, etc.
- **SEG macro**: Added a macro to construct segment descriptors with appropriate bit settings.
- **STA_X, STA_W, STA_R**: Added constants for segment type bits (executable, writable, readable).
- **STS_T32A**: Removed the available 32-bit TSS definition.
- **Removed definitions**: Commented out or removed several page table related constants like `PGADDR`, `NPDENTRIES`, `NPTENTRIES`, `PGSIZE`, `PTE_P`, etc., as they are not used in this segmentation-focused update.

## Impact
These changes shift the focus from paging to segmentation, providing the infrastructure for setting up GDT entries for kernel and user code/data segments. The `SEG` macro simplifies creating segment descriptors.