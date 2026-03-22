# defs.h Changes: New Function Declarations and Forward References

## Summary
Extended the global function declarations header to expose new kernel capabilities for process management, CPU state control, and virtual memory management.

## Changes in Detail

### 1. Forward Declaration of `struct proc`

**Added:**
```c
struct proc;
```

**Location:** Near the top of the file, with other forward declarations

**Why This Is Important:**

Forward declarations allow files to reference `struct proc` without including the full definition from `proc.h`. This prevents circular dependencies.

#### Benefits:
- **Type Safety**: Compiler knows `struct proc` exists
- **Avoid Circular Includes**: Prevents `proc.h` from including `console.c` or other files
- **Compile Time**: Faster compilation (doesn't parse full struct definition)

#### Example of Problem Without It:
```c
// In defs.h - WITHOUT forward declaration
// To declare: void switchuvm(struct proc*);
// We'd need to: #include "proc.h"

// But proc.h might include defs.h
// Result: CIRCULAR DEPENDENCY

// Error: struct proc already has partial or complete definition
```

#### With Forward Declaration:
```c
// In defs.h - WITH forward declaration
struct proc;  // Promise: this type exists elsewhere

// Now we can safely use it:
void switchuvm(struct proc*);  // OK - pointer to unknown type is fine
```

**Key Point:** You can use a forward-declared type in **pointers and references**, but not for:
- Accessing struct members
- Knowing the size of the struct
- Creating stack instances

---

### 2. Added `procdump()` Declaration

**Added:**
```c
void            procdump(void);
```

**Location:** In the "proc.c" section after `scheduler()` declaration

**Purpose:** Declares the process debugging function that prints all running processes.

**Function Signature:**
```c
void procdump(void)
  ↓
// Returns nothing
// Takes no parameters
```

**Typical Implementation:**
```c
void
procdump(void)
{
  struct proc *p;
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    int i;
    cprintf("%d %s %s", p->pid, states[p->state], p->name);
    // Print more info...
  }
}
```

**When It's Called:**
- User presses **Ctrl+P** in the console
- `consoleintr()` sets the `doprocdump` flag
- After input processing, `procdump()` is called asynchronously

---

### 3. Added `pushcli()` and `popcli()` Declarations

**Added:**
```c
void            pushcli(void);
void            popcli(void);
```

**Location:** In the "spinlock.c" section

#### Purpose: Manage CPU Interrupt State

These functions protect critical sections in the kernel from being interrupted.

#### How They Work:

```c
pushcli();        // Disable interrupts, save old state
// ... critical section - kernel data is safe from interrupts ...
popcli();         // Restore previous interrupt state
```

#### Implementation Pattern (typical):
```c
// Before: interrupts might be ON or OFF
pushcli();  // Turn OFF interrupts, remember they were ON
{
  // Critical section - interrupts disabled
  // Safe to modify shared kernel data structures
}
popcli();   // Restore to original state (ON if they were ON)
// After: interrupts in original state
```

#### Why This Is Needed:

```
Multi-Core Problem:

CPU 0: Reading process count from ptable
  ↓
(INTERRUPT - CPU 1 modifies ptable)
  ↓
CPU 0: Reads corrupted data (partially updated count)
  ↓
RACE CONDITION!
```

Solution with `pushcli()` / `popcli()`:
```
CPU 0: pushcli()
  ↓
(Interrupts disabled - CPU 1 can't interrupt)
  ↓
CPU 0: Reading process count from ptable (safe!)
  ↓
CPU 0: popcli()
  ↓
(Interrupts re-enabled)
  ↓
SAFE!
```

#### Nesting Support:
These functions support **nested calls**:
```c
pushcli();      // Level 1: Disable interrupts (depth = 1)
  pushcli();    // Level 2: Already disabled, inc depth (depth = 2)
    // ... protected code ...
  popcli();     // Level 2: Dec depth (depth = 1, still disabled)
popcli();       // Level 1: Dec depth (depth = 0, restore original)
```

#### Real Usage Example:
```c
// In proc.c - scheduler function
void
scheduler(void)
{
  struct proc *p;

  for(;;){
    pushcli();  // Disable interrupts during process selection
    
    // Safely iterate process table
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;
      
      // Schedule this process
      break;
    }
    
    popcli();   // Re-enable interrupts
  }
}
```

---

### 4. Added `switchuvm()` Declaration

**Added:**
```c
void            switchuvm(struct proc*);
```

**Location:** In the "vm.c" section after `seginit()`

**Purpose:** Switch the current CPU's user virtual memory context to a specific process

#### What It Does:

When the CPU switches from running Process A to Process B, it must:

```
Before switchuvm(process_B):
  CPU's memory translation table points to Process A's address space
  CPU executes Process A's code
  
  ↓ (Context switch decision made by scheduler)
  
After switchuvm(process_B):
  CPU's memory translation table points to Process B's address space
  CPU executes Process B's code in isolated memory
```

#### Memory Isolation Achieved:

```
Process A's View:
  Virtual Address 0x1000 → Physical Address 0x200000
  Virtual Address 0x2000 → Physical Address 0x201000

Process B's View:
  Virtual Address 0x1000 → Physical Address 0x300000  (different!)
  Virtual Address 0x2000 → Physical Address 0x301000
```

#### Implementation Concept:
```c
void
switchuvm(struct proc *p)
{
  if(p == 0)
    panic("switchuvm: no process");
  
  // Switch CPU's page directory to this process's
  lcr3(V2P(p->pgdir));  // Load page directory register
  
  // Update CPU's Task State Segment (TSS) for this process
  mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
  mycpu()->ts.ss0 = SEG_KDATA << 3;
  
  // Verify the switch worked
  if(mycpu()->gdt[0] != 0){
    // ... validation ...
  }
}
```

#### Why This Is Critical for Lab 3 (Scheduler):

Your scheduler will use `switchuvm()` to implement process switching:

```c
// In your scheduler implementation
void
scheduler(void)
{
  struct proc *p;

  for(;;){
    // Find a runnable process
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state == RUNNABLE){
        // Switch to this process's memory space
        switchuvm(p);
        
        // Run the process via context switch
        swtch(&mycpu()->context, p->context);
        
        // Return here after process yields
        switchuvm(0);
      }
    }
  }
}
```

---

## Summary Table

| Function | Purpose | Safety Feature | Lab 3 Usage |
|----------|---------|-----------------|------------|
| `struct proc` forward declaration | Enable use of process type pointers | Avoids circular deps | Type-safe process pointers |
| `procdump()` | Print all running processes for debugging | Called safely asynchronously | Verify correct scheduling |
| `pushcli()` / `popcli()` | Protect critical sections from interrupts | Prevents race conditions | Safe process table access |
| `switchuvm()` | Switch CPU to process's memory space | Memory isolation | Context switching between processes |

---

## Dependency Graph

```
console.c ──calls──> procdump()
                        ↓
                    [Uses process table]
                        ↓
                    Needs: struct proc
                    Needs: pushcli/popcli (for safety)

scheduler() ──uses──> switchuvm()
    ↓                  ↓
    Modifies ptable    Switches CPU memory context
    ↓
    Needs: pushcli/popcli (safe access)
```

---

## Testing Checklist for Lab 3

- [ ] Verify `procdump()` prints correctly with Ctrl+P
- [ ] Verify `pushcli()`/`popcli()` don't deadlock
- [ ] Verify `switchuvm()` correctly isolates process memory
- [ ] Verify processes can only access their own memory
- [ ] Verify scheduler correctly uses all three functions

---

## Related Concepts for Lab 3

### Race Condition Without pushcli/popcli:
```c
// UNSAFE (without interrupt protection)
int count = 0;
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
  if(p->state != UNUSED)
    count++;  // RACE! Interrupt here corrupts count
}
```

### Why Virtual Memory Matters:
- Process A can't read Process B's data (memory isolation)
- Process A can't see kernel memory accidentally
- xv6 kernel memory is always accessible (privilege level 0)
- User code operates at privilege level 3 (restricted)

### Ready for Lab 3?
Once you understand these four pieces, you're ready to implement scheduling with proper isolation and safety guarantees!
