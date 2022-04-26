// Project 2 for CS 4348.005 Operating Systems Concepts Spring 2022
// Authors: Alex Wan, Isabelle Villegas, Leonel Perez

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>  //includes system calls for reading/writing files
#include <fcntl.h>   //includes constants useful for manipulating files
#include <assert.h>  //TESTING: allows use of assert() macro
#include <time.h>    //used to create and update timestamps
#include <cstring>   //contains functions for string comparison, to parse user inputs
#include <stdlib.h>  //contains the atoi() function to get information from the user
#include <iostream>  //for debugging and user interactivity
#include <fstream>   //for file manipulation
#include <stdexcept> //for throwing and handling exceptions

#include "./structures.h" //contains the definitions for superblock, i-node, directory, etc.
#include <unistd.h>       // required for read command
#include <sys/types.h>    // required for lseek
#include <fcntl.h>

///////////////////////////////
/// FUNCTION PROTOTYPES/////////
///////////////////////////////
inode_type inode_reader(int, inode_type);
void inode_writer(int, inode_type);

void removeDirectoryEntry(inode_type inode, int index);
dir_type getDirectoryEntry(inode_type inode, int index);
void addDirectoryEntry(inode_type inode, dir_type entry, int index);

///////////////////////////////
// GLOBAL VARS ////////////////
///////////////////////////////

char fileSystemPath[100];
char pwd[100];
int curINodeNumber;
int fd;                     // file descriptor of the file containing the filesystem
superblock_type superBlock; // the current superblock, stored in memory
dir_type directory;
bool ready; // indicates whether the filesystem is ready for use

////////////Leo CODE
void writeToBlock(int blockNumber, void *buffer, int nbytes)
{
    lseek(fd, (BLOCK_SIZE * blockNumber), SEEK_SET); // Gets us to the block we want
    write(fd, &buffer, nbytes);                      // Writes in the selected block from above with whats
}

void addFreeBlock(int blockNumber)
{
    if (superBlock.nfree == 200) // Free array is full
    {
        // write to the new block
        writeToBlock(blockNumber, superBlock.free, 200 * sizeof(unsigned int));
        superBlock.nfree = 0; // Resets nfree to 0 to begin filling up new blocks
    }
    else
    {
        superBlock.free[superBlock.nfree] = blockNumber;
        superBlock.nfree++;
    }
}

// releases the specified inode
void addFreeInode(int iNodeNumber)
{
    // ensure the inode is marked unallocated
    inode_type inode = inode_reader(iNodeNumber, inode);
    if (inode.flags & ALLOCATED)
        inode.flags -= ALLOCATED;

    // attempt to add the inode to the free list
    if (superBlock.ninode == 200)
    {
        return;
    }
    else
    {
        superBlock.inode[superBlock.ninode] = iNodeNumber;
        superBlock.ninode++;
    }
}

// returns the inode address of an inode
int getInode()
{
    int nodeNum;
    if (superBlock.ninode > 0)
    { // there are inodes left in the free i-node array
        superBlock.ninode--;
        nodeNum = superBlock.inode[superBlock.ninode];

    } /* else {
         //TODO: if the array is empty, repopulate with unallocated inodes from the i-blocks
     }*/
    return nodeNum;
}

int getFreeBlock()
{
    if (superBlock.nfree == 0)
    { // if the free list is empty
        int blockNumber = superBlock.free[0];
        lseek(fd, BLOCK_SIZE * blockNumber, SEEK_SET);
        read(fd, superBlock.free, 200 * sizeof(unsigned int));
        superBlock.nfree = 200;
    }
    // subtracts a block from the free list and returns it
    superBlock.nfree--;
    return superBlock.free[superBlock.nfree];
}

void writeToBlockOffset(int blockNumber, int offset, void *buffer, int nbytes)
{
    lseek(fd, (1024 * blockNumber) + offset, SEEK_SET);
    write(fd, &buffer, nbytes);
}

//  creates the root directory
void createRootDirectory()
{

    int blockNumber = getFreeBlock();

    // intitializes directory entry with 2 spots
    dir_type first, second;

    // The first spot in the directory gets inode = 0 with fileName "."
    first.inode = 0;
    strncpy(first.filename, ".", 2);

    // The second spot in the directory gets inode = 0 aswell with fileName ".."
    second.inode = 0;
    strncpy(second.filename, "..", 3);

    // Creates object of the inode type
    inode_type root;

    // set flags
    root.flags = ALLOCATED + DIRECTORY;
    root.nlinks = 1;
    root.uid = 0;
    root.gid = 0;
    root.size0 = 0;
    root.size1 = 2 * sizeof(dir_type);
    root.addr[0] = blockNumber;
    root.actime = time(NULL);
    root.modtime = time(NULL);

    addDirectoryEntry(root, first, 0);
    addDirectoryEntry(root, second, 1);

    inode_writer(0, root);
    curINodeNumber = 0;
    strcpy(pwd, "/");
}

/*
 * initfs()
 * Initializes the filesystem on file filename, with fsize total blocks and isize i-node blocks
 * Returns a file descriptor pointing to the new file, or -1 if the file could not be created
 */
int initfs(const char *filename, int totalDataBlks, int totaliNodeBlks)
{
    const int inodesPerBlock = BLOCK_SIZE / sizeof(inode_type);
    fprintf(stderr, "Initializing filesystem\n");

    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if ((fd = open(filename, O_RDWR | O_CREAT, 0600)) == -1)
    {
        fprintf(stdout, "Failure to open File");
        return 0;
    }

    strcpy(fileSystemPath, filename);

    //(1)
    // initiate fsize
    superBlock.fsize = totalDataBlks;
    char empty_Block[1024] = {0};
    // initiate isize (Number of blocks for inodes)
    superBlock.isize = (totaliNodeBlks * 64) / 1024;

    //(2)
    // writing empty block to last Datablock
    writeToBlock(totalDataBlks - 1, empty_Block, 1024);
    // Add all blocks to the free array
    superBlock.nfree = 0;
    // Initializes the regular datablocks to the blocks after the last i-node and then adds them to them to the free list
    for (int dataBlockNumber = superBlock.isize + 1; dataBlockNumber < totalDataBlks; dataBlockNumber++)
    {
        addFreeBlock(dataBlockNumber);
    }

    //(3)
    // add free Inodes to inode array
    superBlock.ninode = 0; // ninode is the # of free i-nodes in the i-node array
    // iNodeNumber starts at 1 becuase iNodeNumber 0 is for the root directory
    for (int iNodeNumber = 1; iNodeNumber <= totaliNodeBlks * inodesPerBlock; iNodeNumber++)
        addFreeInode(iNodeNumber);

    //(4)   //Sets these flags for the superBlock
    superBlock.flock = 'f';
    superBlock.ilock = 'i';
    superBlock.fmod = 'f';

    //(5)
    // write Super Block to block 1
    // becuase superBlock starts at block 1 and boot block at block 0
    writeToBlock(1, &superBlock, 1024); //??????????????

    //(6)
    // create empty i-node blocks
    // i-nodes start at block 2
    for (int i = 2; i <= superBlock.isize; i++)
        writeToBlock(i, empty_Block, 1024);

    //(7)
    createRootDirectory();

    return fd;
}

// End of Leo Code

/*
 * cpin()
 * create a new file called "fileName" in the v6 file system and fill
 * the contents of the newly created file with the contents of the externalfile
 *
 * returns inode address of the new file
 */
int cpin(const char *extfile, const char *fileName)
{
    int fd2;
    if ((fd2 = open(extfile, O_RDWR | O_CREAT, 0600)) == -1)
    {
        printf("\n File did not open correctly");
        return -1;
    }
    else
    {
        inode_type newNode;
        newNode.flags = 0;
        newNode.nlinks = 0;
        newNode.uid = 0;
        newNode.gid = 0;
        newNode.size0 = 0;
        newNode.size1 = 0;
        newNode.actime = time(NULL);
        newNode.modtime = time(NULL);
        for (int counter = 0; counter < 9; counter++)
        {
            newNode.addr[counter] = 0;
        }

        // split the file contents into blocks and write to the system
        char buffer[BLOCK_SIZE];
        int addrIndex = 0, bytesRead = BLOCK_SIZE;
        while (bytesRead == BLOCK_SIZE && addrIndex < 9)
        {
            bytesRead = read(fd2, buffer, BLOCK_SIZE);
            newNode.addr[addrIndex] = getFreeBlock();
            lseek(fd, newNode.addr[addrIndex], SEEK_SET);
            write(fd, buffer, BLOCK_SIZE);
            addrIndex++;
        }

        //  create a directory entry
        dir_type newEntry;
        newEntry.inode = getInode();
        strncpy(newEntry.filename, fileName, sizeof(newEntry.filename));
        //  put the file in root directory
        inode_type root = inode_reader(0, root);
        addDirectoryEntry(root, newEntry, 2);

        // write the inode
        int inode_address = getInode();
        inode_writer(inode_address, newNode);

        return inode_address;
    }
}

/* TODO
 * cpout()
 * If the v6-file exists, create externalfile and make the externalfile's
 * contents equal to v6-file.
 */
int cpout(const char *fileName, const char *extFile)
{
    return 0;
}

/* TODO
 * rm()
 * Remove the v6-file from the v6File system
 */
int rm(const char *fileName)
{
    inode_type rootdir = inode_reader(0, rootdir);
    dir_type entry;
    for (int counter = 0; counter < BLOCK_SIZE / sizeof(dir_type); counter++)
    {
        entry = getDirectoryEntry(rootdir, counter);
        if(strcmp(fileName, entry.filename) == 0) {
            inode_type file = inode_reader(entry.inode, file);
            for(int i = 0; i < file.size0; i+=BLOCK_SIZE) {
                addFreeBlock(file.addr[i/BLOCK_SIZE]);
            }
            addFreeInode(entry.inode);
            dir_type empty_entry;
            removeDirectoryEntry(rootdir, counter);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INODE FUNCTIONS //////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Function to write inode, from Professor's jumpstart file
void inode_writer(int inum, inode_type inode)
{
    lseek(fd, 2 * 1024 + (inum)*64, SEEK_SET);
    write(fd, &inode, sizeof(inode));
}

// Function to read inodes, from Profesor's jumpstart file
inode_type inode_reader(int inum, inode_type inode)
{
    lseek(fd, 2 * 1024 + (inum)*64, SEEK_SET);
    read(fd, &inode, sizeof(inode));
    return inode;
}
/////////////////////////
// DIRECTORY FUNCTIONS //
/////////////////////////

// returns the directory entry at the given index
dir_type getDirectoryEntry(inode_type inode, int index)
{
    // ensure the inode is a directory
    if (inode.flags & DIRECTORY)
    {
        //  translate from logical entry number to physical address
        const int dirCapacity = BLOCK_SIZE / sizeof(dir_type);
        int blocknum = inode.addr[0];

        // read the directory entry
        dir_type entry;
        lseek(fd, BLOCK_SIZE * blocknum + index * sizeof(dir_type), SEEK_SET);
        read(fd, &entry, sizeof(dir_type));

        return entry;
    }
    else
    {
        throw std::invalid_argument("Error: argument to getDirectoryEntry() must be a directory.");
    }
    dir_type error;
    error.inode = -1;
    return error;
}

// adds the entry to the directory at index index
void addDirectoryEntry(inode_type inode, dir_type entry, int index)
{
    if (inode.flags & DIRECTORY)
    {
        // convert index to logical inode address
        const int dirCapacity = BLOCK_SIZE / sizeof(dir_type);
        int blocknum = index / dirCapacity;
        // convert logical inode block to physical address
        blocknum = inode.addr[blocknum];
        // write the directory entry
        lseek(fd, BLOCK_SIZE * blocknum + (index * sizeof(dir_type)), SEEK_SET);
        write(fd, &entry, sizeof(dir_type));

        return;
    }
    else
    {
        throw std::invalid_argument("Error: argument to addDirectoryEntry() must be a directory.");
    }
}

void removeDirectoryEntry(inode_type inode, int index) {
    const int dirCapacity = BLOCK_SIZE / sizeof(dir_type);
    int blocknum = index / dirCapacity;
    // convert logical inode block to physical address
    blocknum = inode.addr[blocknum];

    lseek(fd, BLOCK_SIZE * blocknum + (index * sizeof(dir_type)), SEEK_SET);
    write(fd, (char[sizeof(dir_type)]){ }, sizeof(dir_type));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GENERAL SYSTEM FUNCTIONS//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void exit()
{ // saves and closes the filesystem
    if (ready)
    {
        close(fd);
        ready = false;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TESTING USER INPUT /////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
int main()
{
    ready = false;
    bool running = true;
    char input[64] = {" "};
    char* token;

    while (running){    //get user input
        fprintf(stdout, "Enter a command:\n");
        scanf(" %[^\n]s", input);
        token = strtok(input, " ");

        if (strcmp(token, "q") == 0){   //exit the program
            fprintf(stdout, "Exiting v6 filesystem\n");
            exit();
            running = false;
        } else if (strcmp(token, "initfs") == 0){ //initialize the filesystem
            char* filename;
            int fsize = 0, isize = 0;

            filename = strtok(NULL, " ");
            fsize = atoi(strtok(NULL, " "));
            isize = atoi(strtok(NULL, " "));
            initfs(filename, fsize, isize);
        }
        //creates a new file called v6-file in the v6 file system
        //and fill the contents of the newly created file with the
        //contents of the externalfile
        else if(strcmp(token, "cpin") == 0){
            char* filename;
            char* extFile;

            extFile = strtok(NULL, " ");
            filename = strtok(NULL, " ");
            cpin(extFile, filename);
        }
        //if the v6-file exists, create externalfile and make
        //the externalfile's contents equal to v6-file
        else if(strcmp(token, "cpout") == 0){ //
            char* filename;
            char* extFile;

            filename = strtok(NULL, " ");
            extFile = strtok(NULL, " ");
            cpout(filename, extFile);
        }
        //will delete the file v6_file from the v6 file system.
        //Remove all the data blocks of the file, free the
        //i-node and remove the directory entry.
        else if(strcmp(token, "rm") == 0){ //
            char* filename;
            filename = strtok(NULL, " ");
            rm(filename);
        }
        else {    //command was not recognized
            fprintf(stdout, "Command not recognized, enter a new command\n");
        }
    }

    return 0;
}
*/

/////////////////////////////////////////////////////////////////////////////////
// SYSTEM TESTING MAIN FUNCTION /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int main()
{
    int fsystem = initfs("my-v6", 50, 10);
    // verifying root directory integrity
    inode_type rootdir = inode_reader(0, rootdir);
    std::cout << "root flags: " << rootdir.flags << "\n";
    std::cout << "address of root directory: " << rootdir.addr[0] << "\n";
    // add file to root directory
    cpin("test.txt", "test");
    //  get contents of root directory
    std::cout << "Files in root directory:\n";
    dir_type entry;
    for (int counter = 0; counter < BLOCK_SIZE / sizeof(dir_type); counter++)
    {
        entry = getDirectoryEntry(rootdir, counter);
        std::cout << "Entry " << counter << ": " << entry.inode << " " << entry.filename << "\n";
    }
    std::cout << "copying garbage to filesystem\n";
    cpin("garbage.txt", "garbage");
    //  get contents of root directory
    std::cout << "Files in root directory:\n";
    for (int counter = 0; counter < BLOCK_SIZE / sizeof(dir_type); counter++)
    {
        entry = getDirectoryEntry(rootdir, counter);
        std::cout << "Entry " << counter << ": " << entry.inode << " " << entry.filename << "\n";
    }
    std::cout << "removing garbage file\n";
    rm("garbage");
    //  get contents of root directory
    std::cout << "Files in root directory:\n";
    for (int counter = 0; counter < BLOCK_SIZE / sizeof(dir_type); counter++)
    {
        entry = getDirectoryEntry(rootdir, counter);
        std::cout << "Entry " << counter << ": " << entry.inode << " " << entry.filename << "\n";
    }
}