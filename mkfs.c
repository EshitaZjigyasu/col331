#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#include "param.h"

#ifndef static_assert
#define static_assert(a, b) do { switch (0) case 0: case (a): ; } while (0)
#endif

#define NINODES 200

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblocks = NINODES / IPB + 1;
int nlog = LOGSIZE;
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks

int fsfd; // file descriptor
struct superblock sb;
char zeroes[BSIZE];
uint freeinode = 1;
uint freeblock;


void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);

// convert to intel byte order (little endian)
ushort
xshort(ushort x) // stores 16bit in little endian
{
  ushort y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}

uint
xint(uint x) // stores 32 bit in little endian
{
  uint y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

int
main(int argc, char *argv[]) // creates disk image fs.img
{
  int i, cc, fd;
  uint rootino, inum, off;
  struct dirent de; // directory entry ie. a file or '.' or '..'
  char buf[BSIZE];
  struct dinode din;


  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");

  if(argc < 2){
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  assert((BSIZE % sizeof(struct dinode)) == 0);
  assert((BSIZE % sizeof(struct dirent)) == 0);

  fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
  if(fsfd < 0){
    perror(argv[1]);
    exit(1);
  }

  // 1 fs block = 1 disk sector
  nmeta = 2 + nlog + ninodeblocks + nbitmap;
  nblocks = FSSIZE - nmeta;

  sb.size = xint(FSSIZE);
  sb.nblocks = xint(nblocks);
  sb.ninodes = xint(NINODES);
  sb.nlog = xint(nlog);
  sb.logstart = xint(2);
  sb.inodestart = xint(2+nlog);
  sb.bmapstart = xint(2+nlog+ninodeblocks);

  printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
         nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

  freeblock = nmeta;     // the first free block that we can allocate

  for(i = 0; i < FSSIZE; i++) // initialize all blocks to 0
    wsect(i, zeroes);

  memset(buf, 0, sizeof(buf)); // clears buffer by writing zeroes
  memmove(buf, &sb, sizeof(sb)); // populate the buffer with the sb struct 
  wsect(1, buf); // now write this buffer (which contains the superblock) to block number 1 of the disk (since block 0 has bootloader) 

  rootino = ialloc(T_DIR); // allocates a new directoy inode which will be the first inode to be created on disk
  assert(rootino == ROOTINO); // root inode number should be 1

  bzero(&de, sizeof(de)); // 1. Clear a directory entry struct
  de.inum = xshort(rootino);  // 2. Set the inode number (here, it's 1 since '.' is also the root directory only)
  strcpy(de.name, "."); // 3. Set the name to be "."
  iappend(rootino, &de, sizeof(de)); // 4. Append this entry to the root directory

  bzero(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, "..");
  iappend(rootino, &de, sizeof(de));

  for(i = 2; i < argc; i++){
    assert(index(argv[i], '/') == 0);

    if((fd = open(argv[i], 0)) < 0){
      perror(argv[i]);
      exit(1);
    }

    // Skip leading _ in name when writing to file system.
    // The binaries are named _rm, _cat, etc. to keep the
    // build operating system from trying to execute them
    // in place of system binaries like rm and cat.
    if(argv[i][0] == '_')
      ++argv[i];

    inum = ialloc(T_FILE);

    bzero(&de, sizeof(de));
    de.inum = xshort(inum);
    strncpy(de.name, argv[i], DIRSIZ);
    iappend(rootino, &de, sizeof(de));

    while((cc = read(fd, buf, sizeof(buf))) > 0)
      iappend(inum, buf, cc);

    close(fd);
  }

  // fix size of root inode dir
  rinode(rootino, &din);
  off = xint(din.size);
  off = ((off/BSIZE) + 1) * BSIZE;
  din.size = xint(off);
  winode(rootino, &din);

  balloc(freeblock);

  exit(0);
}

void
wsect(uint sec, void *buf) // write sector (used to write a single block of data to the file system image file it is creating)
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){ // move file pointer to sec * BSIZE
    perror("lseek");
    exit(1);
  }
  if(write(fsfd, buf, BSIZE) != BSIZE){ // write BSIZE bytes from buf to the current position of file pointer
    perror("write");
    exit(1);
  }
}

/*
Disk drives read and write in blocks (sectors), usually 512 bytes. However, an inode is much smaller than a block (typically 64 bytes in xv6).

This means one disk block contains multiple inodes (specifically IPB or "Inodes Per Block").

You cannot just "write an inode" to the disk. You must:
1. Read the whole block containing the inode you want to change (so you don't overwrite its neighbors).
2. Find the exact location (dip) of your specific inode inside that block of data.
3. Update just that struct in memory.
4. Write the whole block back to the disk.
*/
void
winode(uint inum, struct dinode *ip) // write inode
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip; // disk inode pointer

  bn = IBLOCK(inum, sb); // get the block number of the inode inum using the superblock sb
  rsect(bn, buf); // read the block into the buffer
  // CRITICAL STEP:
  // 1. Cast 'buf' (raw bytes) to a 'struct dinode*' array.
  // 2. Add an offset to point to the specific inode within that block.
  dip = ((struct dinode*)buf) + (inum % IPB); 
  *dip = *ip; // Copy the new inode data (*ip) into that specific spot in the buffer (*dip)
  wsect(bn, buf); // Write the modified block back to disk
}

void
rinode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *ip = *dip;
}

void
rsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(read(fsfd, buf, BSIZE) != BSIZE){
    perror("read");
    exit(1);
  }
}

uint
ialloc(ushort type) // inode allocate
{
  uint inum = freeinode++; // assign the next free inode
  struct dinode din; // on disk inode structure

  bzero(&din, sizeof(din)); // clears memory to ensure no garbage data is written
  din.type = xshort(type);
  din.nlink = xshort(1);
  din.size = xint(0);
  winode(inum, &din); //
  return inum;
}

void
balloc(int used)
{
  uchar buf[BSIZE];
  int i;

  printf("balloc: first %d blocks have been allocated\n", used);
  assert(used < BSIZE*8);
  bzero(buf, BSIZE);
  for(i = 0; i < used; i++){
    buf[i/8] = buf[i/8] | (0x1 << (i%8));
  }
  printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
  wsect(sb.bmapstart, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

void
iappend(uint inum, void *xp, int n) // inode append
{
  char *p = (char*)xp;
  uint fbn, off, n1;
  struct dinode din;
  char buf[BSIZE];
  uint indirect[NINDIRECT];
  uint x;

  rinode(inum, &din); // 1. Read the inode to get current size and block addresses
  off = xint(din.size); // 2. Get current file size (offset where we start writing since we are appending to the directory's inode to add a file)
  // printf("append inum %d at off %d sz %d\n", inum, off, n);
  while(n > 0){
    fbn = off / BSIZE; // 3. Calculate "File Block Number" (0th block, 1st block, etc. ie. block number inside the file)
    assert(fbn < MAXFILE);
    if(fbn < NDIRECT){ 
      if(xint(din.addrs[fbn]) == 0){ // file block address of 0 means that no disk block has been assigned to that block of the file
        din.addrs[fbn] = xint(freeblock++); // freebock is maintained in mkfs.c which tracks the next free block on the disk image
      }
      x = xint(din.addrs[fbn]);
    } else { // indirect block
      if(xint(din.addrs[NDIRECT]) == 0){
        din.addrs[NDIRECT] = xint(freeblock++); // Allocate indirect block itself
      }
      rsect(xint(din.addrs[NDIRECT]), (char*)indirect); // read the entire indirect block
      if(indirect[fbn - NDIRECT] == 0){
        indirect[fbn - NDIRECT] = xint(freeblock++); // Allocate data block inside indirect block
        wsect(xint(din.addrs[NDIRECT]), (char*)indirect); // Save indirect block back
      }
      x = xint(indirect[fbn-NDIRECT]); // Get the actual data block number
    }
    // now we are actually writing the data into the disk blocks
    /*
    Goal: Calculate how many bytes (n1) we can write into the current block before it gets full.
    n: The total number of bytes remaining to be written.
    fbn: The current "File Block Number" (0, 1, 2...).
    off: The absolute byte offset in the file where we are currently writing (e.g., byte 520).
    (fbn + 1) * BSIZE: This calculates the byte offset of the end of the current block.
    If fbn is 1 and BSIZE is 512, the end is at (1+1)*512 = 1024.
    ... - off: This subtracts our current position from the end of the block.
    If off is 520, then the remaining space in this block is 1024 - 520 = 504 bytes.
    min(...): We take the smaller of "bytes remaining to write" (n) and "space remaining in this block". If we have 1000 bytes to write (n=1000) but only 504 bytes left in the block, n1 becomes 504. We will loop around for the rest.
    */
    n1 = min(n, (fbn + 1) * BSIZE - off);
    /*
    Goal: Read the existing content of the target disk block (x) into memory (buf).
    Why? We might be appending to a partially filled block.
    Example: A file has 50 bytes. We want to append 10 more. The block x already contains 50 bytes of valid data followed by zeroes.
    If we just wrote our new 10 bytes to disk without reading first, we would overwrite the whole 512-byte sector with just our 10 bytes (and garbage/zeros elsewhere), destroying the first 50 bytes!
    By reading first, we preserve the existing data.
    */
    rsect(x, buf);
    /*
    Goal: Copy our new data into the correct spot within the memory buffer.
    p: Pointer to our source data (the stuff we want to write).
    buf: The 512-byte buffer representing the disk block.
    off - (fbn * BSIZE): This calculates the offset within the block.
    If off is 520 and we are in block 1 (fbn=1, start=512), the relative offset is 520 - 512 = 8.
    So we write to buf[8].
    n1: The number of bytes to copy (calculated in step 1).
    */
    bcopy(p, buf + off - (fbn * BSIZE), n1);
    /*
    Goal: Write the modified 512-byte buffer back to the physical disk sector x.
    Now the disk has the original data (preserved by rsect) plus our new data (added by bcopy).
    */
    wsect(x, buf);
    n -= n1;
    off += n1;
    p += n1;
  }
  din.size = xint(off); // Update the file size in the inode
  winode(inum, &din); // Save the updated inode to disk
}
