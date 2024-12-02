#include "c4.h"

#ifdef FILE_PC
// Support for files
#define BLOCK_CACHE_SZ 16
#define BLOCK_DIRTY    0x01
#define BLOCK_CLEAN    0xFE;

cell inputFp, outputFp;
typedef struct { ushort num; byte seq, flags; char data[BLOCK_SZ]; } CACHE_T;
CACHE_T blockCache[BLOCK_CACHE_SZ];
extern char *toIn;

static void dumpBlock(const char *msg, CACHE_T *p) {
    zTypeF("\n%s: blk:%d, seq:%d, flgs:%d",msg, p->num, p->seq, p->flags);
}

static void readBlock(CACHE_T *p) {
    // dumpBlock("read", p);
    for (int i=0; i<BLOCK_SZ; i++) { p->data[i] = 0; }
    cell fh=fileOpen("blocks.fth","rb");
    if (fh) {
        cell req = p->num*BLOCK_SZ;
        fileSeek(fh, req);
        cell pos = ftell((FILE*)fh);
        // zTypeF("-req:%d,pos:%d-", req, pos); key();
        if (req == pos) { fileRead(p->data, BLOCK_SZ, fh); }
        else { zType("-error reading block-"); }
        fileClose(fh);
    }
    p->flags = 0;
}

void writeBlock(CACHE_T *p) {
    dumpBlock("write", p);
    cell fh=fileOpen("blocks.fth","r+b");
    if (!fh) { fh=fileOpen("blocks.fth", "wb"); }
    if (fh) {
        cell req = p->num*BLOCK_SZ;
        fileSeek(fh, req);
        cell pos = ftell((FILE*)fh);
        zTypeF("-req:%d,pos:%d-", req, pos);
        if (pos == req) { fileWrite(p->data, BLOCK_SZ, fh); }
        else { zType("-error writing block-"); }
        fileClose(fh);
    }
    p->flags &= BLOCK_CLEAN;
}

void flushBlocks() {
    for (int i=0; i<BLOCK_CACHE_SZ; i++) {
        CACHE_T *p = &blockCache[i];
        if (p->flags & BLOCK_DIRTY) { writeBlock(p); }
    }
}

static CACHE_T *cached(int blk) {
    // zTypeF("find: %d", blk);
    CACHE_T *p=NULL;
    byte min=255, max=0, cur=255;
    if (blk == 0) { return NULL; }
    for (int i=0; i<BLOCK_CACHE_SZ; i++) {
        CACHE_T *q = &blockCache[i];
        if (q->num == blk) { return q; } // Found
        if (q->num == 0) { p=q; break; } // Unused
        if (q->seq < min) { p=q; min=p->seq; } // Oldest
        if (q->seq > max) { max=q->seq; } // Newest
    }

    if (p->flags & BLOCK_DIRTY) { writeBlock(p); }
    p->num = blk;
    p->seq = max+1;
    p->flags = 0;
    readBlock(p);
    // dumpBlock("find", p);
    return p;
}

char *blockAddr(cell blk) { return blk ? &cached(blk)->data[0] : NULL; }
void blockLoad(int blk) { if (blk) { inPush(toIn); toIn=blockAddr(blk); toIn[BLOCK_SZ-1]=0; } }
void blockLoadNext(int blk) { if (blk) { toIn=blockAddr(blk); toIn[BLOCK_SZ-1]=0; } }
void blockDirty(int blk) { CACHE_T *p=cached(blk); if (p) { p->flags |= BLOCK_DIRTY; } }

FILE *makeIn(cell fh) { return fh ? (FILE*)fh: stdin; }
FILE *makeOut(cell fh) { return fh ? (FILE*)fh: stdout; }

cell fileOpen(const char *name, const char *mode) { return (cell)fopen(name, mode); }
void fileClose(cell fh) { fclose((FILE*)fh); }
void fileDelete(const char *name) { remove(name); }
cell fileRead(char *buf, int sz, cell fh) { return fread(buf, 1, sz, makeIn(fh)); }
cell fileWrite(char *buf, int sz, cell fh) { return fwrite(buf, 1, sz, makeOut(fh)); }
cell fileSeek(cell fh, cell pos) { return fseek((FILE*)fh,pos,SEEK_SET); }
void fileInit() {}

#endif // FILE_PC
