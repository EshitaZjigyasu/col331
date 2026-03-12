# File System Core Updates (`fs.c`)

The changes in `fs.c` modify the core read and write functions (`readi` and `writei`) to dispatch operations to device drivers when interacting with device files.

## 1. `readi` Modification
When reading an inode, `readi` now checks if the inode represents a device (`T_DEV`).

```c
// In readi(struct inode *ip, ...)
if(ip->type == T_DEV){
  // Check for valid major device number and registered read function
  if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
    return -1;
    
  // Dispatch control to the device-specific read function (e.g., consoleread)
  return devsw[ip->major].read(ip, dst, n);
}
```

## 2. `writei` Modification
Similarly, `writei` delegates writing to the device driver if the target inode is a device.

```c
// In writei(struct inode *ip, ...)
if(ip->type == T_DEV){
  // Check for valid major device number and registered write function
  if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
    return -1;
    
  // Dispatch to device write function (e.g., consolewrite)
  return devsw[ip->major].write(ip, src, n);
}
```

This redirection allows standard file I/O system calls (`read`, `write`) to transparently work with hardware devices.
