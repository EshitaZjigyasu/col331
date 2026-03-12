# Miscellaneous Changes and Documentation

## Documentation Updates

### `p13-dev-fs.md` (New File)
A new documentation file explaining the concept of interacting with devices as files (device files), similar to Unix/Linux philosophy. It highlights how `console.c` exposes read/write methods and maps them to `struct devsw`.

### `diff_11_12.md` (Deleted)
The previous plan/diff documentation file was removed.

### `p12-log.md`
Minor text corrections in the logging documentation to fix grammar ("is used to check" vs "checks") and clarifications ("idempotent").

## Minor Code Cleanups

### `log.c`
Updated a comment to be more concise.
```c
// called at the start of each FS system call.
```

### `mkfs.c`
Updated a comment style.
```c
// convert to intel byte order
```
