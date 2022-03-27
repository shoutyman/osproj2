#include <stdio.h>    
#include <stdbool.h>
#include "./structures.h" //contains the definitions for superblock, i-node, directory, etc.

/*
* TODO: initfs()
* Initializes the filesystem on file filename, with fsize total blocks and isize i-node blocks
*/
int initfs(char* filename, int fsize, int isize) {
    return 0;
}

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

void add_free_block(int); // Method we need to modify
void get_free_block(); // Method we need to modify