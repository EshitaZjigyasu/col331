# swtch.S (New File)

## Overview
Provides the assembly routine for switching CPU contexts between processes.

## Implementation Details
```asm
.globl swtch
swtch:
  movl 4(%esp), %eax    # new context pointer

  # Switch stacks
  movl %eax, %esp
  movl $0, %eax

  # Restore callee-saved registers from new context stack
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  ret
```

## Impact
This function is the low-level mechanism used by the scheduler to resume a different process. The caller must arrange for the old context to be saved on its stack before invoking `swtch`.