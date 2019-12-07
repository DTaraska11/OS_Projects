//
// Created by domin on 12/4/2019.
//




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include "disk.h"
#include "disk.c"




#define DISK_BLOCKS 16384
#define BLOCK_SIZE 4096

static int active = 0;  /* is the virtual disk open (active) */
static int handle;      /* file handle to virtual disk       */


typedef struct iNode {

    _Bool isUsed;
    _Bool isDir;
    int parentDir;
    int inodeNum;
    char name[12];
    char extension[4];
    int fileSize;
    char creationDate[25], lastMod[25];
    int dataPointer[12];

}iNode;

typedef struct dataStruct {
    char data[32000000];
}dataStruct;

typedef struct superBlock {

    int root;
    int blockSize;
    int diskBlocks;
    int maxFileSize;
    int numInodeBlocks;
    int numInodes;
    int nodesinBlock;
    int dBitMap;
    int iBitMap;
    int inodes;
    int data;
    int dBitMapSize;
    int iBitMapSize;
    int dataSize;
    int inodeSize;

}superBlock;

typedef struct iNodeBitMap {

    _Bool isUsed[BLOCK_SIZE];
}iNodeBitMap;

typedef struct dataBitMap{

    _Bool isUsed[BLOCK_SIZE];
}dataBitMap;


iNode *inode[100];
iNode *currentDir;
superBlock *supBlock;
iNodeBitMap *iMap;
dataBitMap *dmap;
dataStruct *dat;





int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int umount_fs(char *disk_name);


int fs_open(char *filename);
int fs_create(char *name);
int fs_delete(char *filename);
int fs_mkdir(char *filename);
int fs_read(char *filename);
int getInode(iNodeBitMap *iMap);
int getDataBlock(dataBitMap *dmap);
int createInode(char *name, int dLocation, int freeNode, _Bool dir);
int update_dir(int inode);
int fs_close();
int fs_write(char *filename);
void list();

int make_fs(char *disk_name){

    make_disk(disk_name);
    open_disk(disk_name);

    //mapping of structures
    supBlock = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);


    iMap = mmap(NULL, sizeof(iNodeBitMap), PROT_READ | PROT_WRITE, MAP_SHARED, handle, BLOCK_SIZE);

    dmap = mmap(NULL, sizeof(dataBitMap), PROT_READ | PROT_WRITE , MAP_SHARED , handle, BLOCK_SIZE*2);

    for(int i = 0; i < 100; i++) {
        inode[i] = mmap(NULL, sizeof(iNode), PROT_READ | PROT_WRITE , MAP_SHARED , handle, (BLOCK_SIZE * 3) + (BLOCK_SIZE * i));

    }

    dat = mmap(NULL, sizeof(dataStruct), PROT_READ | PROT_WRITE , MAP_SHARED , handle, BLOCK_SIZE * 104);


    //initializations

    supBlock->blockSize = BLOCK_SIZE;
    supBlock->diskBlocks = DISK_BLOCKS;
    supBlock->maxFileSize = 32000000;
    supBlock->nodesinBlock = 4;
    supBlock->numInodeBlocks = 25;
    supBlock->numInodes = 100;
    supBlock->iBitMap = BLOCK_SIZE;
    supBlock->dBitMap = BLOCK_SIZE * 2;
    supBlock->inodes = BLOCK_SIZE * 3;
    supBlock->data = BLOCK_SIZE * 104;
    supBlock->iBitMapSize = sizeof(iNodeBitMap);
    supBlock->dBitMapSize = sizeof(dataBitMap);
    supBlock->inodeSize = sizeof(iNode);
    supBlock->dataSize = sizeof(dataStruct);
    supBlock->root = supBlock->inodes;

    for(int i = 0; i < BLOCK_SIZE; i++) {
        iMap->isUsed[i] = 0;
    }

    for(int i = 0; i < BLOCK_SIZE; i++) {
        dmap->isUsed[i] = 0;
    }

    for(int i = 0; i < 100; i++) {
        inode[i]->isUsed = 0;
        inode[i]->isDir = 0;
        inode[i]->parentDir = -1;
        inode[i]->inodeNum= -1;
        inode[i]->fileSize = -1;
        for(int j = 0; j < 12; j++) {
            inode[i]->dataPointer[j] = -1;
        }
    }

    fs_create("root");

    currentDir = inode[0];

    return 0;
}

int mount_fs(char *disk_name){



    return 0;
}


int fs_mkdir(char *filename) {
    //find free inode
    int free_node = getInode(iMap);


    int dblock = getDataBlock(dmap);
    int dataAddr = (dblock * BLOCK_SIZE + supBlock->data);

    //initialize inode
    createInode(filename, dataAddr, free_node, 1);

    //update directory
    if(update_dir(free_node)) {
        perror("Directory full");
    } else {
        printf("Directory '%s' successfully created\n", filename);
    }
}

int getInode(iNodeBitMap *iMap) {
    for(int x = 0; x < BLOCK_SIZE; x++) {
        if(!iMap->isUsed[x]) {
            iMap->isUsed[x] = 1;
            return x;
        }
    }
    return -1;
}

int getDataBlock(dataBitMap *dmap) {

    for(int x = 0; x < BLOCK_SIZE; x++) {
        if(!dmap->isUsed[x]) {
            dmap->isUsed[x] = 1;
            return x;
        }
    }
    return -1;
}

int createInode(char *name, int dLocation, int freeNode, _Bool dir){
    inode[freeNode]->isUsed = 1;
    inode[freeNode]->isDir = 1;

    strcmp(name, "root") == 0 ?
    (inode[freeNode]->parentDir = 0) :
    (inode[freeNode]->parentDir = currentDir->inodeNum);

    inode[freeNode]->inodeNum = (unsigned) freeNode;
    inode[freeNode]->fileSize = 0;
    strcpy(inode[freeNode]->name, name);

    dir ?
    strcpy(inode[freeNode]->extension, "dir") :
    strcpy(inode[freeNode]->extension, "txt");


    inode[freeNode]->dataPointer[0] = (unsigned) dLocation;

}


int update_dir(int inode) {
    for(int x = 0; x < 12; x++) {
        if(currentDir->dataPointer[x] < 0) {
            currentDir->dataPointer[x] = inode;
            return 0;
        }
    }
    return 1;
}
int fs_create(char *name){

    int freeNode = getInode(iMap);
    int freeDataBlock = getDataBlock(dmap);
    int dLocation = (freeDataBlock * BLOCK_SIZE + supBlock->data);
    createInode(name, dLocation, freeNode, 0);
    if(strcmp(name, "root") != 0) {
        if(update_dir(freeNode)) {
            printf("No space");
            return -1;
        } else {

            return 0;
        }
    }

}

int fs_open(char *filename) {
    for(int i = 0; i < 100; i++) {
        if(strcmp(filename, inode[i]->name) == 0) {
            if(inode[i]->isDir) {
                currentDir = inode[i];
                printf("Opened '%s'\n", filename);

                return 0;
            }
            else {
                perror("File is not a directory");
            }

        }
    }
    perror("File not found");
    return 1;
}

int fs_delete(char *filename) {
    for(int i = 0; i < 100; i++) {
        if(strcmp(filename, inode[i]->name) == 0) {
            int index = i;

            
            iMap->isUsed[index] = 0;

            
            inode[index]->isUsed = 0;

            
            int j = 0;
            int dblocks = 0;
            while(inode[index]->dataPointer[j] > 0 && j < 12) {
                dblocks++;
                j++;
            }

            
            while(dblocks >= 0) {
                dmap->isUsed[dblocks] = 0;
                dblocks--;
            }
            return 0;
        }
    }
    perror("File not found");
    return 1;
}

int fs_close() {
    if(strcmp(currentDir->name, "root") == 0) {
        perror("Can't close root directory");
        return 1;
    }

    
    int y = currentDir->parentDir;

    
    currentDir = inode[y];

    puts("Folder closed.");

    return 0;
}

int fs_read(char *filename) {
    for(int i = 0; i < 100; i++) {
        if(strcmp(filename, inode[i]->name) == 0) {
            FILE *filesys = fdopen(handle, "w+");
            char buf[BLOCK_SIZE];
            iNode *read = inode[i];
            int j = 0;

            
            while(read->dataPointer[j] >= 0 && j < 12) {
                fseek(filesys, read->dataPointer[j], SEEK_SET);

                fread(buf, BLOCK_SIZE, 1, filesys);
                printf("%s", buf);
                j++;
            }
            return 0;
        }
    }
    perror("File not found");
    return 1;
}

int fs_write(char *filename) {
    for(int i = 0; i < 100; i++) {
        if(strcmp(filename, inode[i]->name) == 0) {
            char buf[BLOCK_SIZE];
            FILE *filesys = fdopen(handle, "w+");
            iNode *read = inode[i];

            
            puts("Please enter data to write:");
            fgets(buf, 512, stdin);
            

            fseek(filesys, read->dataPointer[0], SEEK_SET);
            fwrite(buf, BLOCK_SIZE, 1, filesys);


            fsync(handle);
            puts("Successful write.");
            return 0;
        }
    }
    perror("File not found");
    return 1;
}

void list() {
    int y = 0;

    
    while(currentDir->dataPointer[y] >=0 && y < 12) {
        int inode2 = currentDir->dataPointer[y];
        printf("%s", inode[y]->name);
        y++;
    }
}

int getFile(char *userInput, char name[]) {
    int x = 0;

    
    while((!(isspace(userInput[x]))) && userInput[x] != '\0') {
        x++;
    }

   
    if(!isspace(userInput[x])) return 1;
    userInput++;

    int dirptr = 0;

    
    while(userInput[x] != '\0' && dirptr < 12) {
        name[dirptr++] = userInput[x++];
    }

    name[dirptr] = '\0';

    return 0;
}
void shell(){

    char *userInput = malloc(sizeof(userInput));
    char file[12];
    int finished = 0;



    printf("------------------------------------------------ \n");
    printf("Operations supported:  \n");
    printf("mkdir \n");
    printf("create \n");
    printf("open \n");
    printf("delete \n");
    printf("close \n");
    printf("read \n");
    printf("write \n");
    printf("list \n");
    printf("exit \n");
    printf("------------------------------------------------ \n");



    while(!finished){

        
        fgets(userInput, 128, stdin);

        
        if((strlen(userInput) > 0) && (userInput[strlen(userInput) - 1] == '\n')) {
            userInput[strlen(userInput) - 1] = '\0';
        }

        
        if(strncmp(userInput, "mkdir", 5) == 0) {
            
            if(getFile(userInput, file)) {

                
                printf("Enter directory name: \n ");
                continue;
            }

            fs_mkdir(file);

        }else if(strncmp(userInput, "create", 7) == 0) {
            if(getFile(userInput, file)) {
                printf("Enter filename: \n");
                continue;
            }

            fs_create(file);

        }else if(strncmp(userInput, "open", 4) == 0) {
            if(getFile(userInput, file)) {
                printf("What file would you like to open: \n");
                continue;
            }
            fs_open(file);

        }else if(strcmp(userInput, "close") == 0) {
            fs_close();

        }else if(strncmp(userInput, "read", 4) == 0) {
            if(getFile(userInput, file)) {
                printf("Enter file to read:  \n");
                continue;
            }
            fs_read(file);

        }else if(strncmp(userInput, "write", 5) == 0) {
            if(getFile(userInput, file)) {
                printf("Enter file to write to: \n");
                continue;
            }
            fs_write(file);

        }else if(strncmp(userInput, "delete", 6) == 0) {
            if(getFile(userInput, file)) {
                printf("Enter file to delete: \n ");
                continue;
            }
            fs_delete(file);

        }else if(strcmp(userInput, "list") == 0) {
            list();

        }else if(strcmp(userInput, "exit") == 0) {
            finished = 1;

        }else {
            printf("Operation not recognized.\n");
        }


    }
    free(userInput);

}





