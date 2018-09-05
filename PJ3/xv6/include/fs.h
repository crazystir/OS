#ifndef _FS_H_
#define _FS_H_

// On-disk file system format.
// Both the kernel and user programs use this header file.

// Block 0 is unused.
// Block 1 is super block.
// Inodes start at block 2.

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size
#define DEV_NUM 2

// File system super block
// The devInodeTagsNum stores the inode number of the table stores the map
// (inode number of file, inode number of the file that saved the tag and value corresponding to the former file)
// The value devInodeTagsSize saves the size of the devInodesTagsNum
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint devInodeTagsNum[DEV_NUM];
  int devInodeTagsSize[DEV_NUM];
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i)     ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

#define TAG_NAME_LENGTH 10
#define TAG_SIZE (512 * 128)
#define ALL_TAG_INUM 4096
#define TAG_INUM 200
#define TAG_START (512 * 11)

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

struct inodeTag {
  uint inum;
  uint tinum;
  int inodeTagsSize;
};

// The tag name and the length of the value, the value is save after the tag head.
struct tag {
  char tagName[TAG_NAME_LENGTH];
  int length;
};

#endif // _FS_H_
