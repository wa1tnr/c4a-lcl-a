#include <cstdio>
#include "c4a.h"

// #define FILE_PICO

#ifdef FILE_PICO

// NOTE: this uses LittleFS for the PICO
#include <LittleFS.h>

#define myFS LittleFS

#define NFILES 1
cell inputFp, outputFp;
static char fn[32];
File files[NFILES+1];
char blockData[NUM_BLOCKS][BLOCK_SZ];

void readBlocks() { } // TODO
void writeBlocks() { } // TODO
char *blockAddr(cell Blk) { return btwi(Blk,0,NUM_BLOCKS) ? &blockData[Blk][0] : 0; }

void fileInit() {
    FSInfo fsinfo;
	zType("\nLittleFS: begin ... ");
	myFS.begin();
	zType("done.");
	zType("\nLittleFS: initialized\n");
    LittleFS.info(fsinfo);
	zTypeF("\nBytes total: %llu, used: %llu", fsinfo.totalBytes, fsinfo.usedBytes);
	inputFp = 0;
	for (int i=0; i<=NFILES; i++) { files[i] = File(); }
	readBlocks();
}


cell fileOpen(const char *fn, const char *mode) {
	int fh = 1;
	if (0 < fh) {
		files[fh] = myFS.open((char*)fn, mode);
        if (files[fh].name() == nullptr) { return 0; }
        if (mode[0] == 'w') { files[fh].truncate(0); }
	}
	return fh;
}

void fileClose(cell fh) {
    if (btwi(fh,1,NFILES)) { files[fh].close(); }
}

void fileDelete(const char *name) {
    myFS.remove(name);
}

cell fileRead(char *buf, int sz, cell fh) {
	return (btwi(fh, 1, NFILES)) ? (int)files[fh].read((uint8_t*)buf, sz) : 0;
}

cell fileWrite(char *buf, int sz, cell fh) {
	return (btwi(fh, 1, NFILES)) ? (int)files[fh].write(buf, sz) : 0;
}


void blockLoad(int blk) { /* TODO */ }
void blockLoadNext(int blk) { /* TODO */ }

#endif // FILE_PICO
