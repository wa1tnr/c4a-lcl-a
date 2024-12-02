#include "c4.h"

#ifndef FILE_NONE
// Support for blocks
#define BLOCK_CACHE_SZ    16
#define BLOCK_DIRTY     0x01
#define BLOCK_CLEAN     0xFE
#define BLOCK_FREE      0xFFFF

typedef struct { uint16_t num, seq, flags; char data[BLOCK_SZ]; } CACHE_T;
CACHE_T blockCache[BLOCK_CACHE_SZ];
static uint16_t seq;
extern char *toIn;

static void dumpCacheEntry(const char *msg, CACHE_T *p) {
    zTypeF("\n%s: blk:%u, seq:%u, flgs:%u",msg, p->num, p->seq, p->flags);
}

void dumpCache() {
    for (int i = 0; i < BLOCK_SZ; i++) { dumpCacheEntry("dump", &blockCache[i]); }
}

void clearCacheEntry(CACHE_T* p) {
    for (int i = 0; i < BLOCK_SZ; i++) { p->data[i] = 0; }
    p->seq = p->flags = 0;
    p->num = BLOCK_FREE;
}

void clearBlockCache() {
    for (int i = 0; i < BLOCK_CACHE_SZ; i++) {
        clearCacheEntry(&blockCache[i]);
    }
    seq = 0;
}

static void readBlock(CACHE_T *p) {
    // dumpCacheEntry("read", p);
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
    dumpCacheEntry("write", p);
    cell fh=fileOpen("blocks.fth","r+b");
    if (!fh) { fh=fileOpen("blocks.fth", "wb"); }
    if (fh) {
        cell req = p->num*BLOCK_SZ;
        fileSeek(fh, req);
        cell pos = ftell((FILE*)fh);
        if (pos == req) { fileWrite(p->data, BLOCK_SZ, fh); }
        else { zType(" -error writing block-"); }
        fileClose(fh);
    }
    p->flags &= BLOCK_CLEAN;
}

void flushBlocks() {
    for (int i = 0; i < BLOCK_CACHE_SZ; i++) {
        CACHE_T* p = &blockCache[i];
        dumpCacheEntry("flush", p);
        if (p->flags & BLOCK_DIRTY) { writeBlock(p); }
    }
    // clearBlockCache();
}

static CACHE_T *findBlock(int blk) {
    // zTypeF("find: %d", blk);
    CACHE_T *p=NULL;
    uint16_t min=0xFFFF;
    if (blk == 0) { return NULL; }
    for (int i=0; i<BLOCK_CACHE_SZ; i++) {
        CACHE_T *q = &blockCache[i];
        if (q->num == blk) { q->seq = ++seq; return q; } // Found
        if (q->num == BLOCK_FREE) { p=q; break; } // Unused
        if (q->seq < min) { p=q; min=p->seq; } // Oldest
    }

    if (p->flags & BLOCK_DIRTY) { writeBlock(p); }
    p->num = blk;
    p->seq = ++seq;
    p->flags = 0;
    readBlock(p);
    // dumpCacheEntry("found", p);
    return p;
}

char *blockAddr(cell blk) { return blk ? &findBlock(blk)->data[0] : NULL; }
void blockLoadNext(int blk) { if (blk) { toIn=blockAddr(blk); toIn[BLOCK_SZ-1]=0; } }
void blockLoad(int blk) { if (blk) { inPush(toIn); blockLoadNext(blk); } }
void blockDirty(int blk) { CACHE_T *p=findBlock(blk); if (p) { p->flags |= BLOCK_DIRTY; } }

#endif // FILE_NONE
