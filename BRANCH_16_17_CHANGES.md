# Branch 16 to 17: Kernel Memory Allocation (kalloc)

## Overview

This branch transition introduces **kernel memory allocation system** (`kalloc`) to xv6. This is a critical subsystem that enables the kernel to dynamically allocate and deallocate memory for process structures, page tables, and other kernel data at runtime.

## Key Features Added

### 1. **Dynamic Memory Allocation for Kernel**
- `kalloc()` - Allocates a 4KB page of kernel memory
- `kfree()` - Frees a previously allocated page
- `kinit()` - Initializes the memory allocator at boot time

### 2. **GDB Debugging Enhancement**
- Re-enabled split-window layout in `.gdbinit.tmpl` for better debugging experience

### 3. **Build System Update**
- Added `kalloc.o` to the kernel object files list

## Why Kernel Memory Allocation Is Critical

### Problem Without kalloc:
```c
// IMPOSSIBLE without kalloc!
struct proc *p = malloc(sizeof(struct proc));  // No malloc in kernel!
```

Without dynamic allocation, the kernel would need to:
- Pre-allocate fixed-size process table (can only run N processes)
- Pre-allocate all page tables at boot (wastes memory)
- Have no way to request more memory as needed

### Solution With kalloc:
```c
// Now possible!
struct proc *p = (struct proc *)kalloc();  // Allocates 4KB page
// ... use process structure ...
kfree((char *)p);  // Free when done
```

## Files Modified

| File | Changes | Purpose |
|------|---------|---------|
| [kalloc.c](./CHANGES_kalloc.c.md) | New file - memory allocator implementation | Provides kernel memory allocation |
| [defs.h](./CHANGES_defs_kalloc.md) | Added function declarations | Expose `kalloc()`, `kfree()`, `kinit()` |
| Makefile | Added `kalloc.o` to OBJS | Include allocator in kernel build |
| `.gdbinit.tmpl` | Uncommented `layout split` | Better debugging UX |

---

## How kalloc Works

### Memory Model:

```
Kernel Virtual Address Space:

┌─────────────────────────────┐
│  Kernel Code/Data (fixed)   │
├─────────────────────────────┤
│  kalloc() Free Page Pool    │  ← Managed by kalloc
│                             │
│  - Linked list of free      │
│  - 4KB page sized chunks    │
│                             │
├─────────────────────────────┤
│  Heap Growth Direction ↓    │
└─────────────────────────────┘
```

### Allocation Process:

```
Call kalloc()
    ↓
Check free list for available page
    ↓
If page available:
  - Remove from free list
  - Return address
    ↓
If no page available:
  - Panic (out of memory)
```

### Deallocation Process:

```
Call kfree(page_address)
    ↓
Verify page is valid
    ↓
Add page back to free list
    ↓
Mark as available for next kalloc()
```

---

## Data Structures

### Memory Allocator State (pseudocode):

```c
// In kalloc.c
struct {
  struct run *freelist;  // Linked list of free pages
} kmem;

// Each free page has a header:
struct run {
  struct run *next;      // Pointer to next free page
  // Remaining space can be used for data
};
```

### Page Size:
- **PGSIZE = 4096 bytes** (standard x86 page size)
- Each `kalloc()` call returns exactly 4KB
- Why? Aligns with hardware page table management

---

## Relationship to Branch 15-16 Concepts

Recall from branch 15-16:
- `switchuvm()` switches the CPU's memory context between processes
- Each process needs its own **page directory** (page table root)
- Before kalloc: No way to allocate these page directories

Now with branch 16-17:
- Process initialization can allocate page directory via `kalloc()`
- Process structures can be dynamically created via `kalloc()`
- True dynamic process creation becomes possible

---

## Integration with Process Management

```
When creating a new process (pinit):

1. Allocate process structure
   struct proc *p = (struct proc *)kalloc();
   
2. Allocate user page directory
   p->pgdir = (pde_t *)kalloc();
   
3. Map user memory into page directory
   
4. Process ready to run!

When process terminates:

1. Free page directory
   kfree((char *)p->pgdir);
   
2. Free process structure
   kfree((char *)p);
```

---

## Testing Strategy

### Verification Checklist:

```bash
# 1. Kernel boots without panic
make qemu
# Should see: "kalloc initialized"

# 2. Multiple processes can be created
# (Each needs memory from kalloc)
# Press Ctrl+P to see process list
# Multiple processes listed = success

# 3. Memory allocated and freed correctly
# (Difficult to test directly, but indirect evidence:
#  - System remains stable
#  - No mysterious memory corruption
#  - Processes can be created and destroyed)
```

---

## Memory Limits

### Before kalloc:
```c
#define NPROC 64  // Fixed - can only create 64 processes
struct proc ptable[NPROC];  // Statically allocated
```

### After kalloc:
```c
// Dynamic allocation within memory constraints
// Realistic limit depends on total available memory
// Example: If 8MB available, kalloc can serve ~2000 pages
```

---

## Critical Section: Interrupt Safety

Since multiple CPUs might call `kalloc()` simultaneously:

```c
void
kalloc_with_safety(void)
{
  pushcli();           // Disable interrupts (from branch 15-16!)
  
  // Check freelist - safe from race conditions
  struct run *r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  
  popcli();            // Re-enable interrupts
  
  return (char *)r;
}
```

This demonstrates **how branch 15-16 concepts enable branch 16-17 safety**.

---

## GDB Debugging Enhancement

### Why `layout split` is useful:

```
With layout split:
┌─────────────────────┬──────────────────┐
│   Assembly Code     │  Registers       │
│   (showing where    │  (CPU state)     │
│    execution is)    │                  │
├─────────────────────┼──────────────────┤
│   GDB Command Line                     │
│   (type commands here)                 │
└─────────────────────┴──────────────────┘
```

Benefits:
- See kernel memory operations in real-time
- Watch registers change during kalloc/kfree
- Visualize memory layout during debugging

---

## Implications for Lab 3 (Scheduler)

Your scheduler will depend on kalloc:

```c
void
pinit(int policy)  // Initialize process with policy (LAB 3)
{
  struct proc *p;
  
  // Allocate process structure
  p = (struct proc *)kalloc();  // Uses kalloc!
  if(!p) {
    panic("pinit: no memory for process");
  }
  
  p->policy = policy;  // Set scheduling policy (LAB 3)
  
  // ... rest of initialization ...
  
  // When process terminates in scheduler:
  kfree((char *)p);  // Free memory
}
```

---

## Summary Table

| Component | Purpose | Enabled By | Enables |
|-----------|---------|-----------|---------|
| `kalloc()` | Allocate 4KB pages | New in 16-17 | Dynamic process creation |
| `kfree()` | Free 4KB pages | New in 16-17 | Memory reuse |
| `kinit()` | Initialize allocator | New in 16-17 | Boot process setup |
| `pushcli/popcli` | Safe critical sections | Branch 15-16 | Safe kalloc operations |
| `switchuvm()` | Switch memory contexts | Branch 15-16 | Process isolation |
| **Lab 3 Scheduler** | Manage processes | All above | Foreground/background scheduling |

---

## Architecture Flow

```
xv6 Boot Sequence:
    ↓
seginit()  [Branch 14]
    ↓
kinit()    [Branch 16-17] ← NEW
    ↓
pinit()    [Can now create processes dynamically]
    ↓
scheduler() [Can switch between processes]
    ↓
User Programs Run
```

---

## Ready for Lab 3?

With kalloc in place, you now have:

✅ **Process structures** can be allocated dynamically  
✅ **Memory management** for kernel data  
✅ **Safe critical sections** via pushcli/popcli  
✅ **Virtual memory switching** via switchuvm()  
✅ **Process creation** is fully functional  

Next step: Implement **priority-based scheduling** with your `set_sched_policy()` and `get_sched_policy()` system calls!
