# Console Driver Updates (`console.c`)

The changes in `console.c` implement the standard file operations (`read` and `write`) for the console device, allowing it to be accessed via the file system interface.

## 1. Includes
Added required headers for file system structures.
```c
#include "fs.h"
#include "file.h"
```

## 2. Console Read Implementation
`consoleread` allows a process to read input from the console buffer (keyboard input).

```c
int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  target = n;
  while(n > 0){
    // Wait for data to be available in input buffer
    while(input.r == input.w);
    
    c = input.buf[input.r++ % INPUT_BUF];
    
    if(c == C('D')){  // EOF Handling (Ctrl+D)
      if(n < target){
        // Save ^D for next time if we've already read some data,
        // to ensure the next read returns 0 (EOF signal) properly.
        input.r--;
      }
      break;
    }
    
    *dst++ = c;
    --n;
    
    // Stop reading on newline (canonical mode behavior)
    if(c == '\n')
      break;
  }

  return target - n; // Return number of bytes read
}
```

## 3. Console Write Implementation
`consolewrite` writes data from a buffer to the console output.

```c
int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff); // Output character by character
  return n;
}
```

## 4. Device Initialization
`consoleinit` registers the console's read and write functions in the global device switch table (`devsw`). This connects the generic file system layer to this specific device driver.

```c
void
consoleinit(void)
{
  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
}
```
