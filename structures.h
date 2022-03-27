/*
* filename: structures.h
* Created on March 27, 2022
* Description:This file contains the definitions of various 
*	filesystem structures including the superblock and i-nodes
*/

const int BLOCK_SIZE = 1024;    //the number of bytes in a block

//directory for i-nodes?
typedef struct directory {
    int isize;
    int fsize;
    int nfree;
    unsigned int free[200]; char flock;
    char ilock;
    char fmod; unsigned int time;
}

//definition of superblock
typedef struct {
    int isize;
    int fsize;
    int nfree;
    unsigned int free[200];
    char flock;
    char ilock;
    char fmod;
    unsigned int time;
} superblock_type; // Block size is 1024 Bytes; not all bytes of superblock
//are used. 

//definition of i-node
typedef struct inode {
    unsigned short flags;
    unsigned short nlinks;
    unsigned int uid;
    unsigned int gid;
    unsigned int size0;
    unsigned int size1;
    unsigned int addr[9];
    unsigned int actime;
    unsigned int modtime;
} inode_type; //64 Bytes in size

typedef struct {
    unsigned int inode;
    char filename[28];
} dir_type;  //32 Bytes long