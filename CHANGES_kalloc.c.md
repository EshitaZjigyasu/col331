# kalloc.c: Kernel Memory Allocation Implementation

## Overview

`kalloc.c` implements a simple but effective **free-list based memory allocator** for the kernel. It allows the kernel to dynamically allocate and deallocate 4KB pages of memory at runtime.

## Core Concept: Free List Allocation

Rather than using complex data structures, kalloc uses the simplest possible approach: a **linked list of free memory pages**.

### Basic Idea:

```
Free Memory Pages (linked list):

┌──────────┐     ┌──────────┐     ┌──────────┐
│  Page 1  │ --> │  Page 2  │ --> │  Page 3  │ --> NULL
│ [run*]   │     │ [run*]   │     │ [run*]   │
└──────────┘     └──────────┘     └──────────┘
     ↑
  kmem.freelist
```

Each page starts with a `struct run` containing:
- **next**: Pointer to the next free page
- **remaining space**: Available for allocated data

---

## Data Structures

### struct run:
```c
struct run {
  struct run *next;  // Link to next free page
};
```

**Size:** Only 4 bytes on 32-bit x86 (one pointer)

**Clever trick:** The `run` structure header lives *inside* the free page itself!

```
A free 4KB page:

┌─────────────────────────────────────────────┐
│ struct run (4 bytes)                        │  ← "next" pointer
│ ┌───────────────────────────────────────┐   │
│ │  Unused space (4092 bytes)            │   │← Can be used when allocated
│ └───────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
     0                                      4096
```

When allocated, the `run` header is overwritten with user data.

### struct kmem (kernel memory state):
```c
struct {
  struct run *freelist;  // Head of free page list
} kmem;
```

---

## The Three Key Functions

### 1. `kinit(void *vstart, void *vend)`

**Purpose:** Initialize the allocator at boot time

**Input Parameters:**
- `vstart` - Virtual address of start of free memory
- `vend` - Virtual address of end of free memory

**What It Does:**

```c
void
kinit(void *vstart, void *vend)
{
  // Align start address to next page boundary
  // Example: 0x80105000 → 0x80106000
  
  char *p = (char *)PGROUNDUP((uint)vstart);
  
  // Add each page from start to end to free list
  for(; p + PGSIZE <= (char *)vend; p += PGSIZE) {
    kfree(p);  // Use kfree to add to list
  }
}
```

**Memory Layout at Boot:**

```
Before kinit():
┌─────────────────────────┐
│  Kernel Code (4MB)      │
├─────────────────────────┤
│  Used Data Structures   │
├─────────────────────────┤ ← vstart (unaligned)
│  Free Memory (8MB)      │
├─────────────────────────┤ ← vend
│  Device Memory          │
└─────────────────────────┘

After kinit():
kmem.freelist → [4KB page 1] → [4KB page 2] → ... → [4KB page N] → NULL
```

**Key Detail - PGROUNDUP:**
```c
#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))

// Rounds UP to next page boundary
// Example: PGROUNDUP(0x105001) = 0x106000
```

---

### 2. `kalloc(void)`

**Purpose:** Allocate a 4KB page of memory for kernel use

**Return Value:**
- Pointer to 4KB page if successful
- NULL if no memory available (rare in practice - kernel panics instead)

**Implementation:**

```c
char*
kalloc(void)
{
  struct run *r;

  // Disable interrupts (for multi-core safety)
  pushcli();
  
  // Get first free page from list
  r = kmem.freelist;
  
  // Remove it from the list
  if(r)
    kmem.freelist = r->next;
  
  // Re-enable interrupts
  popcli();

  return (char *)r;
}
```

**Execution Timeline:**

```
Before kalloc():
kmem.freelist → [Page A: run*|data] → [Page B: run*|data] → NULL

kalloc() call:
  r = Page A's address
  kmem.freelist = Page A's next (which is Page B)

After kalloc():
kmem.freelist → [Page B: run*|data] → NULL
                 (Page A is "allocated", not in free list)

Return: Address of Page A
```

**Usage Pattern:**

```c
// Allocate
struct proc *p = (struct proc *)kalloc();

// Use the memory
p->pid = 1;
p->state = RUNNING;

// Free later
kfree((char *)p);
```

---

### 3. `kfree(char *v)`

**Purpose:** Return a 4KB page to the free list

**Input Parameter:**
- `v` - Virtual address of 4KB page to free

**Implementation:**

```c
void
kfree(char *v)
{
  struct run *r;

  // Validation: page must be page-aligned
  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Clear the page (security/debugging)
  memset(v, 1, PGSIZE);

  // Disable interrupts
  pushcli();
  
  // Add page back to front of free list
  r = (struct run *)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  
  // Re-enable interrupts
  popcli();
}
```

**Execution Timeline:**

```
Before kfree(address):
kmem.freelist → [Page B: run*|data] → NULL

kfree(address_of_Page_A) call:
  r = (struct run *)address_of_Page_A
  r->next = kmem.freelist  (Page B)
  kmem.freelist = r         (Page A)

After kfree():
kmem.freelist → [Page A: run*|data] → [Page B: run*|data] → NULL
                 (Page A is back in free list)
```

**Why Insert at Front (NOT sorted):**
- Simpler implementation (O(1) instead of O(n))
- Fragmentation is not an issue (pages are fixed size)
- Buddy with cache locality (reuses recently freed pages)

---

## Memory Safety Features

### 1. Page Alignment Check:

```c
// In kfree():
if((uint)v % PGSIZE)
  panic("kfree: not aligned");
```

Ensures pointer is exactly on a 4KB boundary:
```
Valid:   0x80100000  (multiple of 4096)
Valid:   0x80101000  (multiple of 4096)
Invalid: 0x80100100  (not multiple of 4096)
```

### 2. Boundary Checking:

```c
// In kfree():
if(v < end || V2P(v) >= PHYSTOP)
  panic("kfree: out of range");
```

Ensures page is within kernel memory range.

### 3. Memset Before Reuse:

```c
// In kfree():
memset(v, 1, PGSIZE);  // Fill with 0x01 pattern
```

**Why?**
- **Security**: Prevents information leakage (old data from freed page)
- **Debugging**: Easy to spot uninitialized data (0x01010101... is distinctive)
- **Bug Detection**: Code using freed memory will read bad values

---

## Multi-Core Safety (Race Conditions)

### Problem Without Protection:

```
CPU 0                          CPU 1
────────────────────────────────────────
kmem.freelist → [A] → [B]
                ↓
r = kmem.freelist  (r = A)
                             kmem.freelist → [B]
                             (CPU 1 just allocated A!)
kmem.freelist = r->next
(Sets to B, A is lost!)
```

### Solution: pushcli() / popcli()

```c
pushcli();  // Disable interrupts

// ATOMIC section - no interruption
r = kmem.freelist;
if(r)
  kmem.freelist = r->next;

popcli();   // Re-enable
```

Guarantees:
- One CPU completes entire operation
- Other CPUs can't interrupt mid-operation
- No race conditions

---

## Real-World Allocation Example

### Scenario: Creating a new process

```
1. scheduler() decides to create process
   
2. Allocate process structure
   struct proc *p = (struct proc *)kalloc();
   
   kmem.freelist → [B] → [C]  (A removed)
   p → address of A

3. Initialize process
   p->pid = 2
   p->state = EMBRYO

4. Allocate user page directory
   p->pgdir = (pde_t *)kalloc();
   
   kmem.freelist → [C]  (B removed)
   p->pgdir → address of B

5. Process runs, uses pages

6. Process terminates, cleanup calls
   kfree((char *)p->pgdir);  // Free B
   
   kmem.freelist → [B] → [C]

7. kfree((char *)p);  // Free A
   
   kmem.freelist → [A] → [B] → [C]
```

---

## Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|-----------------|-------|
| `kalloc()` | O(1) | Get first free page, update pointer |
| `kfree()` | O(1) | Insert at front of list |
| `kinit()` | O(n) | Called once at boot, n = number of pages |

---

## Limitations and Alternatives

### Current Simple Design:
- **Pro**: Minimal code, fast, no fragmentation (fixed size pages)
- **Con**: Can't allocate arbitrary sizes, free list management is simple

### Alternative Designs Not Used Here:
1. **Buddy Allocator**: Supports variable-size allocations but more complex
2. **SLAB Allocator**: Efficient for small objects but overcomplicated for xv6
3. **Virtual Memory Paging**: Not needed since we're allocating physical pages

For xv6's purposes (fixed 4KB pages), the simple list is ideal.

---

## Integration Points

### Where kalloc is Used:

```c
// proc.c - allocate process structure
p = (struct proc *)kalloc();

// vm.c - allocate page directories
p->pgdir = (pde_t *)kalloc();

// Other kernel subsystems needing memory
resource = (type *)kalloc();
```

### When Paired with kfree:

```c
// Always: allocate, use, free
ptr = kalloc();
// ... use ptr ...
kfree(ptr);
```

---

## Debugging Tips

### Check Available Memory:

In GDB:
```gdb
(gdb) print kmem.freelist
$1 = (struct run *) 0x80122000
```

Follow the chain to count free pages:
```gdb
(gdb) print kmem.freelist->next
$2 = (struct run *) 0x80123000
(gdb) print kmem.freelist->next->next
$3 = (struct run *) 0x80124000
// ...
```

### Detect Memory Leaks:

If `kmem.freelist` becomes NULL but processes are still running:
- Memory leak detected
- Some allocated page wasn't freed

### Verify Freed Memory Pattern:

After `kfree()`, memory is filled with 0x01:
```gdb
(gdb) x /16xb 0x80122000
0x80122000:  0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01
0x80122008:  0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01
```

This proves `memset(v, 1, PGSIZE)` was called.

---

## Critical Takeaways

1. **Free List Design**: Each free page contains a pointer to the next free page
2. **Location Efficiency**: The `run` structure lives inside the free page itself
3. **Simplicity**: Only three functions, O(1) operations
4. **Safety**: Uses `pushcli()`/`popcli()` for atomic operations
5. **Fixed Size**: Only allocates/deallocates 4KB chunks (page-sized)

---

## Lab 3 Relevance

Your scheduler needs to:

```c
void
pinit(int policy)  // With your policy parameter
{
  struct proc *p = (struct proc *)kalloc();
  p->policy = policy;  // Your addition from Lab 3
  // ... rest of initialization ...
}
```

kalloc enables dynamic process creation needed for your scheduler to manage multiple processes with different policies!
