//
// Created by domin on 12/3/2019.
//








#ifndef FILESYS_DISK_H
#define FILESYS_DISK_H

#include "filesys.c"

#define DISK_BLOCKS  16384      /* number of blocks on the disk                */
#define BLOCK_SIZE   4096      /* block size on "disk"                        */
/******************************************************************************/
int make_disk(char *name);     /* create an empty, virtual disk file          */
int open_disk(char *name);     /* open a virtual disk (file)                  */
int close_disk();              /* close a previously opened disk (file)       */
int block_write(int block, char *buf);                               /* write a block of size BLOCK_SIZE to disk    */
int block_read(int block, char *buf);                               /* read a block of size BLOCK_SIZE from disk   */
/******************************************************************************/


#endif //FILESYS_DISK_H
