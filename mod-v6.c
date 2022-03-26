#include <stdio.h>    
#include <stdbool.h>

int main()
{ 
    // checks condition of whether or not the 
    // user wants to continue to give commands
    bool x = true; 

    while(x == true){
        char testCommand[25];
        printf("What is your command? ");
        scanf("%s", testCommand);  

       // if(testCommand[]) // test more commands
        if(testCommand[0] == 'q'){
            x = false;
            printf("\rSystem Exited. Thank you.\r");
        }
    }
    return 0;
}

// directory for the i-nodes?
typedef struct directory{
    int isize;
    int fsize;
    int nfree;
    unsigned int free[200]; char flock;
    char ilock;
    char fmod; unsigned int time;
} 
superblock_type; // Block size is 1024 Bytes; not all bytes of superblock //are used.
superblock_type superBlock;
void add_free_block(int); // Method we need to modify
void get_free_block(); // Method we need to modify


// i-node Structure
typedef struct inode{
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
typedef struct { unsigned int inode; char filename[28];
} dir_type;//32 Bytes long