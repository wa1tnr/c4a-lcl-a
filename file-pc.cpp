#include "c4.h"

#ifdef FILE_PC
// Support for files
cell inputFp, outputFp, fileStk[FSTK_SZ + 1];
int fileSp;
byte blocks[NUM_BLOCKS][BLOCK_SZ];
extern char *toIn;

void filePush(cell fh) { if (fileSp < FSTK_SZ) { fileStk[++fileSp] = fh; } }
cell filePop() { return (0 < fileSp) ? fileStk[fileSp--] : 0; }

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
void blockLoad(int blk) { filePush((cell)toIn); toIn=blockAddr(blk); toIn[BLOCK_SZ-1]=0; }
void blockLoadNext(int blk) { toIn=blockAddr(blk); }

void fileInit() { fileSp = 0; readBlocks(); }
cell fileOpen(const char *name, const char *mode) { return (cell)fopen(name, mode); }
void fileClose(cell fh) { fclose((FILE*)fh); }
void fileDelete(const char *name) { remove(name); }
cell fileRead(char *buf, int sz, cell fh) { return fread(buf, 1, sz, (FILE*)fh); }
cell fileWrite(char *buf, int sz, cell fh) { return fwrite(buf, 1, sz, (FILE*)fh); }

int fileGets(char *buf, int sz, cell fh) {
    buf[0] = 0;
    if (fh == 0) { fh = (cell)stdin; }
    if (fgets(buf, sz, (FILE*)fh) != buf) return 0;
    return strLen(buf);
}

void fileLoad(const char *name) {
    cell fh = fileOpen(name, "rt");
    if (fh == 0) { zTypeF("-[%s] not found-", name); return; }
    if (inputFp) { filePush(inputFp); }
    inputFp = fh;
}

#endif // FILE_PC
