/*
 * BSHU2 -- EOS32 file system check
 *
 * Hier ist eine (nicht vollstaendige) Liste von moeglichen Fehlern:
 * [ ] Ein Block ist weder in einer Datei noch auf der Freiliste: Exit-Code 10.
 * [ ] Ein Block ist sowohl in einer Datei als auch auf der Freiliste: Exit-Code 11.
 * [ ] Ein Block ist mehr als einmal in der Freiliste: Exit-Code 12.
 * [ ] Ein Block ist mehr als einmal in einer Datei oder in mehr als einer Datei: Exit-Code 13.
 * [ ] Die Groesse einer Datei ist nicht konsistent mit den im Inode vermerkten Bloecken: Exit-Code 14.
 * [ ] Ein Inode mit Linkcount 0 erscheint in einem Verzeichnis: Exit-Code 15.
 * [ ] Ein Inode mit Linkcount 0 ist nicht frei: Exit-Code 16.
 * [ ] Ein Inode mit Linkcount n != 0 erscheint nicht in exakt n Verzeichnissen: Exit-Code 17.
 * [ ] Ein Inode hat ein Typfeld mit illegalem Wert: Exit-Code 18.
 * [ ] Ein Inode erscheint in einem Verzeichnis, ist aber frei: Exit-Code 19.
 * [ ] Der Root-Inode ist kein Verzeichnis: Exit-Code 20.
 * [ ] Ein Verzeichnis kann von der Wurzel aus nicht erreicht werden: Exit-Code 21.
 * [ ] Alle anderen Dateisystem-Fehler: Exit-Code 99.
 *
 *  Andere moegliche Fehler, die geprueft werden muessen:
 * [x] Falscher Aufruf des Programms: Exit-Code 1.
 * [x] Image-Datei nicht gefunden: Exit-Code 2.
 * [x] Datei Ein/Ausgabefehler: Exit-Code 3.
 * [x] Illegale Partitionsnummer: Exit-Code 4.
 * [x] Partition enthaelt kein EOS32-Dateisystem: Exit-Code 5.
 * [ ] Erfolgloser Aufruf von malloc(): Exit-Code 6.
 * [ ] Alle anderen Fehler: Exit-Code 9.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 512                     //disk sector size in bytes
#define BLOCK_SIZE 4096                     //disk block size in bytes
#define SPB (BLOCK_SIZE / SECTOR_SIZE)      //sectors per block

#define NICINOD 500                         //number of free inodes in superblock
#define NICFREE 500                         //number of free blocks in superblock
#define INOPB 64                            //number of inodes per block
#define DIRPB 64                            //number of directory entries per block
#define DIRSIZ 60                           //max length of path name component

#define IFMT    070000                      //type of file
#define IFREG   040000                      //regular file
#define IFDIR   030000                      //directory
#define IFCHR   020000                      //character special
#define IFBLK   010000                      //block special
#define IFFREE  000000                      //reserved (indicates free inode)

#define ISUID   004000                      //set user id on execution
#define ISGID   002000                      //set group id on execution
#define ISVTX   001000                      //save swapped text even after use
#define IUREAD  000400                      //user's read permission
#define IUWRITE 000200                      //user's write permission
#define IUEXEC  000100                      //user's execute permission
#define IGREAD  000040                      //group's read permission
#define IGWRITE 000020                      //group's write permission
#define IGEXEC  000010                      //group's execute permission
#define IOREAD  000004                      //other's read permission
#define IOWRITE 000002                      //other's write permission
#define IOEXEC  000001                      //other's execute permission


void readSuperBlock(void);
void inspectInodes(void);
void inspectFreelist(void);
void checkBlockCounter(void);

void help(char *);
unsigned int get4Bytes(const unsigned char *);
void readBlock(unsigned int);

typedef struct blockCounter {
    int free;
    int occupied;
} bCounter_t;

FILE *disk;
unsigned int fsStart;
unsigned int fsSize;
int part;
char *endptr;
unsigned char partTable[SECTOR_SIZE];
unsigned char *ptptr;
unsigned int partType;
unsigned char blockBuffer[BLOCK_SIZE];
unsigned char *p;                               //placeholder pointer hold address within the block buffer
unsigned int inodeListSize;
bCounter_t *bCounter;

int main(int argc, char *argv[]) {
    if(argc != 3) {
        //wrong program call
        help(argv[0]);
    }
    disk = fopen(argv[1], "rb");
    if(disk == NULL) {
        //can't open image file
        printf("Error: cannot open disk image file '%s'", argv[1]);
        exit(2);
    }
    if(strcmp(argv[2], "*") == 0) {
        //whole disk contains one single file system
        fsStart = 0;
        fseek(disk, 0, SEEK_END);
        fsSize = ftell(disk) / SECTOR_SIZE;
    } else {
        //argv[2] is partition number of file system
        part = strtoul(argv[2], &endptr, 10);
        if(*endptr != '\0' ||part < 0 || part > 15) {
            printf("Error: illegal partition number '%s'", argv[2]);
            exit(4);
        }
        fseek(disk, 1 * SECTOR_SIZE, SEEK_SET);
        if(fread(partTable, 1 , SECTOR_SIZE, disk) != SECTOR_SIZE) {
            printf("Error: cannot read partition table of disk '%s'", argv[1]);
            exit(3);
        }
        ptptr = partTable + part + 32;
        partType = get4Bytes(ptptr + 0);
        if((partType & 0x7FFFFFFF) != 0x00000058) {
            printf("Error: partition %d of disk '%s' does not contain an EOS32 file system", part, argv[1]);
            exit(5);
        }
        fsStart = get4Bytes(ptptr + 4);
        fsSize = get4Bytes(ptptr + 8);
    }

    bCounter = (bCounter_t *) malloc(sizeof(bCounter_t) * (fsSize/SPB));
    if(bCounter == NULL) {
        printf("Error: Failed malloc() call\n");
        exit(6);
    }

}

void readSuperBlock(void) {

}

void inspectInodes(void) {
    unsigned int i = 2;
    unsigned int j;
    unsigned int mode;
    unsigned int nLink;
    unsigned int size;

    readBlock(1);
    p = blockBuffer;

    p += 8; //skip to inode list size
    inodeListSize = get4Bytes(p);

    while(i < inodeListSize) {
        readBlock(i);
        p = blockBuffer;

        for(j = 0; j < INOPB; j++) {
            mode = get4Bytes(p);
            p += 4;

            if(mode != 0) {
                //TODO: Check if mode is legit
            } else {
                //inode is free
            }

            nLink = get4Bytes(p);
            p += 24;
            size = get4Bytes(p);
            p += 4;



        }

        i++;
    }

    //TODO: go through inodes and count blocks
}

void inspectFreelist(void) {
    //TODO: count blocks appearing on free list
}

void checkBlockCounter(void) {
    //TODO: check that blockCounter for every block has only a value of 1
}

void readBlock(unsigned int blockNum) {
    fseek(disk, fsStart * SECTOR_SIZE + blockNum * BLOCK_SIZE, SEEK_SET);
    if(fread(blockBuffer, BLOCK_SIZE, 1, disk) != 1) {
        printf("Error: cannit read block %u (0x%X)", blockNum, blockNum);
        exit(3);
    }
}

unsigned int get4Bytes(const unsigned char *addr) {
    return (unsigned int) addr[0] << 24 |
           (unsigned int) addr[1] << 16 |
           (unsigned int) addr[2] << 8  |
           (unsigned int) addr[3] << 0;
}

void help(char *name) {
    printf("Usage: %s <disk> <partition>\n", name);
    printf("       <disk> is a disk image file name\n");
    printf("       <partition> is a partition number (or '*' for the whole disk\n");
    exit(1);
}
