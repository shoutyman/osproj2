### Authors:
  Alex Wan,
  Leonel Perez,
  Isabelle Villegas
  
## How to run the file:
  Type make or make all while in the osproj2 file
  Then run ./mod-v6 to run the file
  
## Commands You Can Run:
  **initfs file_name n1 n2** 
  - Will initialize the file system 
      file_name is the name of the file that represents the disk drive. 
      n1 is the file system size in number of blocks
      n2 is the number of blocks devoted to the i-nodes
      
  **cpin externalfile v6-file** 
  - Will create a new file called v6-file in the 
  v6 file system and fill the contents of the newly created file with the contents of the externalfile
  
  **cpout v6-file externalfile** 
  - If the v6-file exists, create externalfile and make the externalfile's contents equal to v6-file
  
  **rm v6-file** 
  - Will delete the file v6_file from the v6 file system. Remove all the data blocks of 
  the file, free the i-node and remove the directory entry.
      
  **q** 
  - Will quit and end and save the program
