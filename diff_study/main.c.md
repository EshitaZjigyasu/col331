# Main System Initialization (`main.c`)

The updates in `main.c` integrate the console device initialization and demonstrate its usage as a file.

## 1. System Initialization
Added calls to initialize the console hardware driver and create the special device file `/console`.

```c
// In main()
consoleinit();   // connect read/write to devsw
// ... (other inits)
iinit(ROOTDEV);  // Initialize inode cache
initlog(ROOTDEV);// Initialize logging

// explicit device file creation
struct inode console;
mknod(&console, "console", CONSOLE, CONSOLE); // major=1, minor=1

welcome();       // uses the new console file
```

## 2. Updated `welcome()`
The `welcome` function has been completely rewritten to demonstrate file I/O on the console device, replacing the previous file system tests (`/foo/hello.txt`).

**New implementation:**
```c
static inline void
welcome(void) {
  struct file* c;

  // Open the "console" device file (created in main)
  if((c = open("console", O_RDWR)) == 0) {
    panic("Failed to open console");
  }
  
  // File I/O operations work on the console!
  filewrite(c, "\nEnter your name: ", 18);
  
  char name[20];
  int namelen = fileread(c, name, 20); // Blocks until input
  
  filewrite(c, "Nice to meet you! ", 18);
  filewrite(c, name, namelen);
  filewrite(c, "BYE!\n", 6);
  
  fileclose(c);
}
```
This shows that `open`, `read`, and `write` operate transparently on devices once they are set up in the VFS layer.
