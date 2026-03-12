# File System Layer Updates (`file.c`)

Changes in `file.c` introduce the `devsw` table and the `mknod` function, which are essential for supporting device files.

## 1. Device Switch Table
Defined the global device switch table. This array holds function pointers for device drivers, indexed by the major device number.

```c
struct devsw devsw[NDEV];
```

## 2. `mknod` Implementation
Implementing `mknod` (make node) to create a new device file. This creates an inode with type `T_DEV` (device) and stores the major and minor device numbers.

```c
int
mknod(struct inode *ip, char* path, int major, int minor)
{
  begin_op(); // Start transaction
  
  // Create a new inode (T_DEV type)
  if((ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  
  iput(ip); // Release the inode
  end_op(); // Commit transaction
  
  return 0;
}
```
Note: `create()` is assumed to handle `T_DEV` and the major/minor numbers correctly (likely in `sysfile.c` or `fs.c` internal logic, though `create` implementation isn't shown changing here, it is used differently).
