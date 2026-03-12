# Header and Definition Updates

Several header files were updated to support device files and the `devsw` table.

## `param.h`
Defined the maximum number of major devices supported.
```c
#define NDEV         10  // maximum major device number
```

## `file.h`
Defined the `devsw` structure which abstracts device operations.
```c
// table mapping major device number to device functions
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1 // Major device number for console
```

## `defs.h`
Added forward declarations for the new functions.
```c
// console.c
void            consoleinit(void);

// file.c
int             mknod(struct inode *ip, char* path, int major, int minor);
```
