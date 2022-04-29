/*
 * filename: structures.h
 * Created on March 27, 2022
 * Description:This file contains the definitions of various
 *	filesystem structures including the superblock and i-nodes
 */

const unsigned int BLOCK_SIZE = 1024; // the number of bytes in a block
const unsigned int INODE_SIZE = 64;

// definition of superblock
typedef struct
{
    int isize;
    int fsize;
    int nfree;
    unsigned short free[200];
    unsigned int ninode;
    unsigned short inode[200];
    char flock;
    char ilock;
    char fmod;
    unsigned int time;
} superblock_type; // Block size is 1024 Bytes; not all bytes of superblock
// are used.

// definition of i-node
typedef struct inode
{
    unsigned short flags;
    unsigned short nlinks;
    unsigned int uid;
    unsigned int gid;
    unsigned int size0; //0;
    unsigned int size1;  //2 * sizeof(dir_type);
    unsigned int addr[9];
    unsigned int actime;
    unsigned int modtime;
} inode_type; // 64 Bytes in size

enum inode_flags
{ // defines values for the 'flags' field of an inode
  // allocated (1)
    ALLOCATED = 32768,

    // file TYPE bits (2 and 3)
    SPECIAL = 24576,
    DIRECTORY = 16384,
    CHAR_SPECIAL = 8192,
    PLAIN = 0,

    // file SIZE bits (4 and 5)
    SMALL = 0,
    MEDIUM = 2048,
    LONG = 4096,
    SUPERLONG = 6144,

    // setuid (6) and setgid(7)
    SETUID = 1024,
    SETGID = 512,

    // permissions for owner (8, 9, 10)
    OWNER_READ = 256,
    OWNER_WRITE = 128,
    OWNER_EXEC = 64,

    // permissions for group (11, 12, 13)
    GROUP_READ = 32,
    GROUP_WRITE = 16,
    GROUP_EXEC = 8,

    // permissions for other (14, 15, 16)
    OTHER_READ = 4,
    OTHER_WRITE = 2,
    OTHER_EXEC = 1
};

typedef struct
{
    unsigned int inode;
    char filename[28];
} dir_type; // 32 Bytes long
