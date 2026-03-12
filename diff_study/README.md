# Diff Study: Branch 12 to 13
This directory contains a detailed breakdown of the changes between branch 12 and 13, focusing on the implementation of Device Files.

## Core Concept: Device Files
The primary goal of these changes is to expose hardware devices (like the console) as files in the file system, allowing standard `read()` and `write()` system calls to interact with them.

## File Breakdown
*   [**console.c**](console.c.md): Implements `consoleread` and `consolewrite` and hooks them into the device switch table.
*   [**file.c**](file.c.md): Adds `mknod` for creating device nodes and defines the `devsw` table.
*   [**fs.c**](fs.c.md): Modifies `readi` and `writei` to redirect operations to device drivers when the inode is a device.
*   [**main.c**](main.c.md): Initialization (`consoleinit`, `mknod`) and usage example (`welcome` function).
*   [**headers.md**](headers.md): Updates to `defs.h`, `file.h`, and `param.h` supporting the new structures.
*   [**misc.md**](misc.md): Minor cleanups and documentation changes.
