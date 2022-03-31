#include <stdio.h>    
#include <stdbool.h>
#include <unistd.h> //includes system calls for reading/writing files
#include <fcntl.h>  //includes constants useful for manipulating files 
#include <assert.h> //TESTING: allows use of assert() macro
#include <time.h>   //used to create and update timestamps

#include "./structures.h" //contains the definitions for superblock, i-node, directory, etc.
#include <unistd.h> // required for read command
#include <sys/types.h> // required for lseek 
#include <fcntl.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARS //////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int fd; //file descriptor of the file containing the filesystem
superblock_type superblock; //the current superblock, stored in memory

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES //////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void add_free_block(unsigned int block); //adds a free block to the filesystem
void write_superblock(int blocknum);//writes superblock object newSuper to the specified block

/*
* initfs()
* Initializes the filesystem on file filename, with fsize total blocks and isize i-node blocks
* Returns a file descriptor pointing to the new file, or -1 if the file could not be created
*/
int initfs(const char* filename = "my_v6", int fsize = 10, int isize = 2) {
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd != -1) { //the file was created successfully    
        //initialize the superblock
        superblock.fsize = fsize;
        superblock.isize = isize;
        superblock.time = static_cast<unsigned int>(time(NULL));

        //create the underlying file: (fsize - 1) empty blocks
        char empty_block[1024] = { 0 };
        for (int counter = 0; counter < fsize; counter++) {
            write(fd, empty_block, BLOCK_SIZE);
        }

        //populate the free[] array
        superblock.free[1] = 0;
        superblock.nfree = 2;
        for (int counter = isize + 2; counter < fsize; counter++) {
            add_free_block(counter);
        }

        //write the superblock as the second block
        write_superblock(1);
    }

    //return the file descriptor of the file for use
    lseek(fd, 0, SEEK_SET);
    return fd;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INODE FUNCTIONS //////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Function to write inode, from Professor's jumpstart file
void inode_writer(int inum, inode_type inode) {

    lseek(fd, 2 * BLOCK_SIZE + (inum - 1) * INODE_SIZE, SEEK_SET);
    write(fd, &inode, sizeof(inode));
}

// Function to read inodes, from Profesor's jumpstart file
inode_type inode_reader(int inum, inode_type inode) {
    lseek(fd, 2 * BLOCK_SIZE + (inum - 1) * INODE_SIZE, SEEK_SET);
    read(fd, &inode, sizeof(inode));
    return inode;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SUPERBLOCK FUNCTIONS /////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  updates the superblock in memory with data from filesystem at block address blocknum
void get_superblock(int blocknum) {
    lseek(fd, blocknum * BLOCK_SIZE, SEEK_SET); 
    read(fd, &superblock, sizeof(superblock_type));
}

//  Function to update filesystem from superblock in memory
void update_superblock() {
    write_superblock(1);
}

//  write_superblock(): copies superblock in memory to specified block
void write_superblock(int blocknum) {
    superblock.time = (unsigned int)time(NULL);
    lseek(fd, blocknum * BLOCK_SIZE, SEEK_SET);
    write(fd, &superblock, sizeof(superblock_type));
}

//  write_superblock(): copies passed superblock object to specified block
void write_superblock(int blocknum, superblock_type block_to_write) {
    block_to_write.time = (unsigned int)time(NULL);
    lseek(fd, blocknum * BLOCK_SIZE, SEEK_SET);
    write(fd, &block_to_write, sizeof(superblock_type));
}

//  outputs the contents of the superblock object in memory
void print_superblock() {
    fprintf(stdout, "Number of blocks: %d\n", superblock.fsize);
    fprintf(stdout, "Number of i-node blocks: %d\n", superblock.isize);
    fprintf(stdout, "Time last modified: %d\n", superblock.time);
}

//adds a block to the free list
void add_free_block(unsigned int block) {
    if (block < superblock.fsize && block >= superblock.isize + 2) {//check filesystem bounds
        if (superblock.nfree == 200) { //current superblock is full; create a new superblock and copy old one to filesystem
            fprintf(stderr, "Superblock capacity exceeded, creating new superblock\n");
            write_superblock(block);
            
            superblock.free[0] = block;
            superblock.nfree = 1;
        }
        else {
            superblock.free[superblock.nfree] = block;
            superblock.nfree++;
            update_superblock();
        }
    }
    else {  //the block is outside the bounds of the filesystem
        fprintf(stderr, "Error: Block is out of bounds\n");
    }
}

//gets the address of a free block from the filesystem referenced by fd
int get_free_block(){
    superblock.nfree--;
    if (superblock.nfree != 0){  //current superblock is not empty
        if (superblock.free[superblock.nfree] == 0){
            fprintf(stderr, "Error: Filesystem empty, could not allocate block\n");
            return -1;
        } else {
            update_superblock();
            return superblock.free[superblock.nfree];
        }
    }
    else {    //the current superblock is empty; fetch new superblock from filesystem
        get_superblock(superblock.free[0]);
        return get_free_block();
    }
    //should never reach this state
    fprintf(stderr, "Unknown error while allocating block");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEST MAIN() FUNCTION /////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{ 
    //initialize the filesystem
    fprintf(stderr, "Initializing v6 filesystem...\n");
    fd = initfs("my_v6", 400, 10);

    int blockNum = get_free_block();
    while (blockNum != -1) {
        blockNum = get_free_block();
        fprintf(stdout, "Block address received: %d\n", blockNum);
    }   

    return 0;
}