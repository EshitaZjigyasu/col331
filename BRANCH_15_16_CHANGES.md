# Branch 15 to 16: Process Debugging and CPU State Management

## Overview

This branch transition introduces **process debugging capabilities** and **CPU state management utilities** to xv6. The main additions enable users to inspect running processes and provide safer interrupt handling mechanisms for critical kernel sections.

## Key Features Added

### 1. **Process Dumping via Keyboard (Ctrl+P)**
- Users can now press **Ctrl+P** to print debugging information about all running processes
- Implemented as an asynchronous handler to avoid deadlock issues

### 2. **Interrupt Safe Critical Sections**
- New functions `pushcli()` and `popcli()` for managing interrupt states
- Essential for protecting shared kernel data structures

### 3. **Process-Aware Virtual Memory Switching**
- New function `switchuvm()` to switch user virtual memory contexts
- Allows the CPU to safely run different processes with their own memory spaces

### 4. **Process Management Declarations**
- Forward declaration of `struct proc`
- Declarations for process-related infrastructure functions

## Files Modified

| File | Changes | Purpose |
|------|---------|---------|
| [console.c](./CHANGES_console.c.md) | Added Ctrl+P handler | Process debugging via keyboard |
| [defs.h](./CHANGES_defs.h.md) | Added function declarations | Expose new kernel capabilities |

## Architecture: How It All Works Together

```
User presses Ctrl+P
    ↓
consoleintr() keyboard handler triggered
    ↓
Sets doprocdump flag (deferred to avoid deadlock)
    ↓
After handling input, calls procdump()
    ↓
procdump() iterates through process table
    ↓
Prints process information (PID, state, name, etc.)
```

## Why These Changes Matter

### Debugging
- Developers can now inspect process state without halting the system
- Critical for diagnosing scheduler and process management issues

### Safety
- `pushcli()` and `popcli()` prevent race conditions in shared kernel code
- Without proper interrupt management, multiple CPUs could corrupt kernel state

### Process Isolation
- `switchuvm()` ensures each process operates in its own isolated address space
- Core feature for process isolation and security

## Related Components

These changes support the upcoming **scheduler implementation** (discussed in Lab 3), which requires:
- Ability to list all processes for scheduling decisions
- Safe CPU state management to prevent race conditions
- Virtual memory switching to run different processes

## Usage Example

### For Lab 3 - Scheduler Implementation

The `procdump()` function will be essential for testing the scheduler:

```c
// Compile and run: make qemu
// Press Ctrl+P in the xv6 terminal to see something like:

pid  ppid  state   name
 1    0    sleep   initcode
 2    1    runnable ioshell
 3    1    runnable background_task
```

This allows you to verify:
- That foreground processes are scheduled more frequently
- That background processes still get CPU time (no starvation)
- That the process table is properly maintained

## Testing Checklist

- [ ] Press Ctrl+P in running xv6 system
- [ ] Verify process list is printed correctly
- [ ] Confirm no deadlock occurs during printing
- [ ] Verify interrupt state is restored after Ctrl+P
- [ ] Test with multiple processes running
