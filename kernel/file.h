struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe; // FD_PIPE
  struct inode *ip;  // FD_INODE and FD_DEVICE
  uint off;          // FD_INODE
  short major;       // FD_DEVICE
};

#define major(dev)  ((dev) >> 16 & 0xFFFF)
#define minor(dev)  ((dev) & 0xFFFF)
#define	mkdev(m,n)  ((uint)((m)<<16| (n)))

// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
  
  // add owner and permissions field
  uint owner;
  uint permissions;
};

// map major device number to device functions.
struct devsw {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

extern struct devsw devsw[];

#define CONSOLE 1

// adding bitmask file permissions
#define READ	4
#define WRITE	2
#define EXEC	1

#define READ_OTHER	4
#define WRITE_OTHER	2
#define EXEC_OTHER	1

#define OWNER_READABLE(ip)    ((ip->permissions >> 2) & READ)
#define OWNER_WRITABLE(ip)    ((ip->permissions >> 1) & WRITE)
#define OWNER_EXECUTABLE(ip)  (ip->permissions & EXEC)
#define OTHER_READABLE(ip)    ((ip->permissions >> 2) & READ_OTHER)
#define OTHER_WRITABLE(ip)    ((ip->permissions >> 1) & WRITE_OTHER)
#define OTHER_EXECUTABLE(ip)  (ip->permissions & EXEC_OTHER)
#define OWNER_PERM(ip)        (ip->permissions >> 3)
#define OTHER_PERM(ip)        (ip->permissions) 
