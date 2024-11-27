#include "c4.h"

#ifdef FILE_PC
// Support for files
cell inputFp, outputFp;
byte blocks[NUM_BLOCKS][BLOCK_SZ];
extern char *toIn;

static void readBlocks() {
    cell fh=fileOpen("blocks.fth","rb");
    if (fh) {
        fileRead((char*)blocks, sizeof(blocks), fh);
        fileClose(fh);
    }
}

void writeBlocks() {
    cell fh=fileOpen("blocks.fth","wb");
    if (fh) {
        fileWrite((char*)blocks, sizeof(blocks), fh);
        fileClose(fh);
    }
}

char *blockAddr(cell blk) { return (char*)&blocks[blk][0]; }
void blockLoad(int blk) { inPush(toIn); toIn=blockAddr(blk); toIn[BLOCK_SZ-1]=0; }
void blockLoadNext(int blk) { toIn=blockAddr(blk); }

FILE *makeIn(cell fh) { return fh ? (FILE*)fh: stdin; }
FILE *makeOut(cell fh) { return fh ? (FILE*)fh: stdout; }

void fileInit() { readBlocks(); }
cell fileOpen(const char *name, const char *mode) { return (cell)fopen(name, mode); }
void fileClose(cell fh) { fclose((FILE*)fh); }
void fileDelete(const char *name) { remove(name); }
cell fileRead(char *buf, int sz, cell fh) { return fread(buf, 1, sz, makeIn(fh)); }
cell fileWrite(char *buf, int sz, cell fh) { return fwrite(buf, 1, sz, makeOut(fh)); }

#endif // FILE_PC
