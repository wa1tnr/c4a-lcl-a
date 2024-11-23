#include <cstdio>
#include "c4.h"

#ifdef FILE_PICO

// NOTE: this uses LittleFS for the PICO
#include <LittleFS.h>

#define myFS LittleFS

#define NFILES 12
cell inputFp, outputFp, fileSp, fileStk[FSTK_SZ+1];
static char fn[32];
File files[NFILES+1];
char blockData[NFILES][1024];

void fileInit() {
    FSInfo fsinfo;
	zType("\nLittleFS: begin ... ");
	myFS.begin();
	zType("done.");
	zType("\nLittleFS: initialized\n");
    LittleFS.info(fsinfo);
	zTypeF("\nBytes total: %llu, used: %llu", fsinfo.totalBytes, fsinfo.usedBytes);
	inputFp = fileSp = 0;
	for (int i=0; i<=NFILES; i++) { files[i] = File(); }
}

static char *blockFn(int blk) { sprintf(fn, "block-%03d.fth", blk); return fn; }
void filePush(cell fh) { if (fileSp < FSTK_SZ) { fileStk[++fileSp] = fh; } }
cell filePop() { return (0 < fileSp) ? fileStk[fileSp--] : 0; }

int freeFile() {
	for (int i=1;i<=NFILES;i++) {
		if ((bool)files[i] == false) { return i; }
	}
	return 0;
}

cell fileOpen(const char *fn, const char *mode) {
	int fh = freeFile();
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

void fileLoad(const char *name) {
    cell fh = fileOpen(name, "r");
    if (fh) {
        char *buf = &blockData[fh-1][0];
        for (int i=0; i<1023; i++) { buf[i]=0; }
        fileRead(buf, 1022, fh);
        outer(buf);
        fileClose(fh);
    }
}

int  fileGets(char *buf, int sz, cell fh) { return 0; }
void blockLoad(int blk) { fileLoad(blockFn(blk)); }
void blockLoadNext(int blk) {}

#endif // FILE_PICO
