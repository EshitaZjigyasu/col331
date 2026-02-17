struct buf;
struct rtcdate;
struct superblock;
struct inode;
struct stat;

// bio.c
void            binit(void);            // Initialize the buffer cache
struct buf*     bread(uint, uint);      // Read a block from disk into the buffer cache
void            bwrite(struct buf*);    // Write a buffer to disk
void            brelse(struct buf *b);  // Release a locked buffer

// console.c
void            cprintf(char*, ...);                  // Print to the console
void            halt(void) __attribute__((noreturn)); // Halt the system
void            panic(char*) __attribute__((noreturn)); // Panic and halt the system
void            consoleintr(int(*)(void));            // Console interrupt handler
void            consputc(int);                        // Output a character to the console

// fs.c
void            readsb(int dev, struct superblock *sb);       // Read the superblock
void            iinit(int dev);                               // Initialize inodes
void            iread(struct inode*);                         // Read content of an inode from disk
struct inode*   iget(uint dev, uint inum);                    // Get an in-memory inode
int             readi(struct inode*, char*, uint, uint);      // Read data from an inode
void            stati(struct inode*, struct stat*);           // Copy inode metadata to stat structure
int             namecmp(const char*, const char*);            // Compare file names
struct inode*   namei(char*);                                 // Lookup an inode by path
struct inode*   nameiparent(char*, char*);                    // Lookup parent directory inode
struct inode*   dirlookup(struct inode*, char*, uint*);       // Lookup a name in a directory
int             dirlink(struct inode *dp, char *name, uint inum); // Create a new directory entry
struct inode*   ialloc(uint dev, short type);                 // Allocate a new inode
void            iupdate(struct inode *ip);                    // Sync in-memory inode to disk

// ide.c
void            ideinit(void);    // Initialize the IDE disk driver
void            ideintr(void);    // IDE disk interrupt handler
void            iderw(struct buf*); // Read/Write a buffer to the disk

// ioapic.c
void            ioapicenable(int irq, int cpu); // Enable an IRQ on a CPU
extern uchar    ioapicid;                       // IOAPIC ID
void            ioapicinit(void);               // Initialize IOAPIC

// lapic.c
void            cmostime(struct rtcdate *r);  // Read time from CMOS
int             lapicid(void);                // Get current CPU's LAPIC ID
extern volatile uint*    lapic;               // LAPIC address
void            lapiceoi(void);               // Acknowledge interrupt to LAPIC
void            lapicinit(void);              // Initialize LAPIC
void            lapicstartap(uchar, uint);    // Start application processors
void            microdelay(int);              // Spin for some microseconds

// mp.c
extern int      ismp;           // Is this a multiprocessor system?
void            mpinit(void);   // Detect other processors

// picirq.c
void            picenable(int); // Enable an interrupt on the PIC
void            picinit(void);  // Initialize the PIC

// proc.c
int             cpuid(void);    // Get current CPU ID
struct cpu*     mycpu(void);    // Get current CPU structure

// spinlock.c
void            getcallerpcs(void*, uint*); // Record the call stack

// string.c
int             memcmp(const void*, const void*, uint);     // Compare memory
void*           memmove(void*, const void*, uint);          // Copy memory (handles overlap)
void*           memset(void*, int, uint);                   // Set memory to a value
char*           safestrcpy(char*, const char*, int);        // Safer string copy
int             strlen(const char*);                        // String length
int             strncmp(const char*, const char*, uint);    // Compare strings (max n characters)
char*           strncpy(char*, const char*, int);           // Copy string (max n characters)

// trap.c
void            idtinit(void);    // Initialize IDT
extern uint     ticks;            // Application ticks
void            tvinit(void);     // Initialize trap vectors

// uart.c
void            uartinit(void);   // Initialize UART
void            uartintr(void);   // UART interrupt handler
void            uartputc(int);    // Output a character to UART

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
