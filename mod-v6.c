#include <stdio.h>    
#include <stdbool.h>
#include <unistd.h> //includes system calls for reading/writing files
#include <fcntl.h>  //includes constants useful for manipulating files 
#include <assert.h> //TESTING: allows use of assert() macro

#include "./structures.h" //contains the definitions for superblock, i-node, directory, etc.
#include <unistd.h> // required for read command
#include <sys/types.h> // required for lseek 
#include <fcntl.h>

/*
* TODO: initfs()
* Initializes the filesystem on file filename, with fsize total blocks and isize i-node blocks
* Returns a file descriptor pointing to the new file, or -1 if the file could not be created
*/
int initfs(const char* filename = "my_v6", int fsize = 10, int isize = 2) {
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd != -1) { //the file was created successfully    
        //initialize the superblock
        superblock_type new_superblock;
        new_superblock.fsize = fsize;
        new_superblock.isize = isize;
        //TODO: implement free-block chain; for now the maximum # of accessible blocks is 200
        if (fsize - (isize + 1) < 200) {
            new_superblock.nfree = fsize - (isize + 1);
        }
        else {
            new_superblock.nfree = 200;
        }
        //populate the free[] array
        for (int counter = 0; counter < new_superblock.nfree; counter++) {
            new_superblock.free[counter] = counter;
        }

        //CREATE THE FILESYSTEM
        //create (fsize - 1) empty blocks
        char empty_block[1024] = { 0 };
        for (int counter = 0; counter < fsize; counter++) {
            write(fd, empty_block, BLOCK_SIZE);
        }
        //write the superblock as the second block
        lseek(fd, BLOCK_SIZE, SEEK_SET);
        write(fd, &new_superblock, sizeof(superblock_type));

    }

    //return the file descriptor of the file for use
    lseek(fd, 0, SEEK_SET);
    return fd;
}

// Function to write inode, from Professor's jumpstart file
void inode_writer(int fd, int inum, inode_type inode) {

    lseek(fd, 2 * BLOCK_SIZE + (inum - 1) * INODE_SIZE, SEEK_SET);
    write(fd, &inode, sizeof(inode));
}

// Function to read inodes, from Profesor's jumpstart file
inode_type inode_reader(int fd, int inum, inode_type inode) {
    lseek(fd, 2 * BLOCK_SIZE + (inum - 1) * INODE_SIZE, SEEK_SET);
    read(fd, &inode, sizeof(inode));
    return inode;
}

//  Function to get the contents of the superblock
superblock_type get_superblock(int fd) {
    superblock_type superblock;
    lseek(fd, BLOCK_SIZE, SEEK_SET);    //superblock is always second block of filesystem
    assert(fd != 0 && fd != 1 && fd != 2);
    read(fd, &superblock, sizeof(superblock));
    return superblock;
}

void update_superblock(superblock_type newsuper) {

}

int main()
{ 
    //initialize the filesystem
    fprintf(stderr, "Initializing v6 filesystem...\n");
    int fd = initfs();
    //get the superblock
    fprintf(stderr, "Fetching superblock...\n");
    superblock_type superblock = get_superblock(fd);
    fprintf(stderr, "Done\n");

    // checks condition of whether or not the 
    // user wants to continue to give commands
    bool x = true;

    while (x == true) {
        char testCommand[25];
        printf("What is your command? ");
        scanf("%s", testCommand);
        // if(testCommand[]) // test more commands
        if (testCommand[0] == 'q') {
            x = false;
            printf("\rSystem Exited. Thank you.\r");
        }
    }
    return 0;
}

void add_free_block(int); // Method we need to modify
void get_free_block(); // Method we need to modify
