#include <cstdio>
#include "c4a.h"

// #define FILE_PICO

#ifdef FILE_PICO

// NOTE: this uses LittleFS for the PICO
#include <LittleFS.h>

#define myFS LittleFS

#define NFILES 16
cell inputFp, outputFp;
// static char fn[32];
static File files[NFILES+1];

static int isValidFile(cell fh) { return btwi(fh, 1, NFILES) ? 1 : 0; }

void fileInit() {
    FSInfo fsinfo;
	zType("\r\nLittleFS: begin ... ");
	myFS.begin();
	zType("done.");
	zType("\r\nLittleFS: initialized\r\n");
    LittleFS.info(fsinfo);
	zTypeF("\r\nBytes total: %llu, used: %llu", fsinfo.totalBytes, fsinfo.usedBytes);
	inputFp = 0;
	for (int i=0; i<=NFILES; i++) { files[i] = File(); }
    blockInit();
}

static cell freeFileSlot() {
	for (int i = 1; i <= NFILES; i++) {
		if ((bool)files[i] == false) { return i; }
	}
	return 0;
}

cell fileOpen(const char *fn, const char *mode) {
	int fh = freeFileSlot();
    zTypeF("-fopen:fh=%d,md=%s-",fh,mode);
	if (fh) {
		files[fh] = myFS.open((char*)fn, mode);
        if (files[fh].name() == nullptr) { return 0; }
        if (mode[0] == 'w') { files[fh].truncate(0); }
	}
	return fh;
}

void fileClose(cell fh) { if (isValidFile(fh)) { files[fh].close(); } }
void fileDelete(const char *name) { myFS.remove(name); }
cell fileSize(cell fh) { return (isValidFile(fh)) ? files[fh].size() : 0; }
cell filePos(cell fh) { return (isValidFile(fh)) ? files[fh].position() : 0; }

cell fileRead(char *buf, int sz, cell fh) {
	return (isValidFile(fh)) ? (int)files[fh].read((uint8_t*)buf, sz) : 0;
}

cell fileSeek(cell fh, cell pos) {
    if (isValidFile(fh)) {
        return files[fh].seek(pos) ? 1 : 0;
    }
    return 0;
}

cell fileWrite(char *buf, int sz, cell fh) {
	return (isValidFile(fh)) ? (int)files[fh].write(buf, sz) : 0;
}

#endif // FILE_PICO
