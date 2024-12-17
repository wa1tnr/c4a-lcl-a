// editor.cpp - A simple block editor

#include "c4a.h"
#include <string.h>

#ifndef EDITOR
void editBlock(cell blk) { zType("-no edit-"); }
#else

#define NUM_LINES     16
#define NUM_COLS      64
#define MAX_LINE      (NUM_LINES-1)
#define MAX_COL       (NUM_COLS-1)
#define EDCH(r,c)     edBuf[((r)*NUM_COLS)+(c)]
#define DIRTY         isDirty=1; isShow=1
#define BCASE         break; case

#ifndef MAX
  #define MIN(a,b) ((a)<(b))?(a):(b)
  #define MAX(a,b) ((a)>(b))?(a):(b)
#endif

enum { NORMAL=1, INSERT, REPLACE, QUIT };
enum { Up=7240, Dn=7248, Rt=7245, Lt=7243, Home=7239, PgUp=7241, PgDn=7249,
    End=7247, Ins=7250, Del=7251, CHome=7287 };

static cell line, off, edMode, isDirty, isShow, block;
static char edBuf[BLOCK_SZ], yanked[NUM_COLS+1], *curLine;

static void GotoXY(int x, int y) { zTypeF("\x1B[%d;%dH", y, x); }
static void CLS() { zType("\x1B[2J"); GotoXY(1, 1); }
static void ClearEOL() { zType("\x1B[K"); }
static void CursorBlock() { zType("\x1B[2 q"); }
static void CursorOn() { zType("\x1B[?25h"); }
static void CursorOff() { zType("\x1B[?25l"); }
static void showCursor() { GotoXY(off+2, line+2); CursorOn(); CursorBlock(); }
static void FG(int fg) { zTypeF("\x1B[38;5;%dm", fg); }
static void toFooter() { GotoXY(1, NUM_LINES+3); }
static void toCmd() { GotoXY(1, NUM_LINES+4); }
static void normalMode()  { edMode=NORMAL;  }
static void insertMode()  { edMode=INSERT;  }
static void replaceMode() { edMode=REPLACE; }
static void toggleInsert() { (edMode==INSERT) ? normalMode() : insertMode(); }
static int winKey() { return (224 << 5) ^ key(); }
static void Green() { FG(40); }
static void Red() { FG(203); }
static void Yellow() { FG(226); }
static void White() { FG(255); }

static void setBlock(int blk) { block=MAX(MIN(blk,BLOCK_MAX),0); storeWC(BLKA, block); }

static int vtKey() {
    int y = key();
    if (y != '[') { return 27; }
    y = key();
    if (btwi(y, 'A', 'T')) {
        switch (y) {
            case 'A': return Up;
            case 'B': return Dn;
            case 'C': return Rt;
            case 'D': return Lt;
            case 'F': return End;
            case 'H': return Home;
            case 'S': return PgUp;
            case 'T': return PgDn;
            default: return 27;
        }
    }
    if (btwi(y, '2', '8')) {
        int z = key();
        if (z!='~') { return 27; }
        switch (y) {
        case '2': return Ins;
        case '3': return Del;
        case '5': return PgUp;
        case '6': return PgDn;
        case '7': return Home;
        case '8': return End;
        default: return 27;
        }
    }
    return 27;
}

static int edKey() {
    int x = key();
    if (x ==  27) { return vtKey(); }    // Possible VT control sequence
    if (x == 224) { return winKey(); }   // Windows: start char
    return x;
}

static void mv(int r, int c) {
    line += r;
    off += c;
    if (line < 0) { line = 0; }
    if (NUM_LINES <= line) { line = MAX_LINE; }
    if (off < 0) { off=0; }
    if (NUM_COLS <= off) { off = MAX_COL;}
}

static void moveWord(int isRight) {
    if (isRight) {
        while ((off < MAX_COL) & (EDCH(line,off) > 32)) { ++off; }
        while ((off < MAX_COL) & (EDCH(line,off) < 33)) { ++off; }
    } else {
        while ((0 < off) & (EDCH(line,off-1) < 33)) { --off; }
        while ((0 < off) & (EDCH(line,off-1) > 32)) { --off; }
    }
}

static void showState(char ch) {
    static int lastState = INTERP;
    int cols[4] = { 40, 203, 226, 255 };
    if (ch == 0) { ch = lastState ? lastState : INTERP; }
    if (btwi(ch,1,5)) { FG(cols[ch-1]); lastState = ch; }
}

static void gotoEOL() {
    off = MAX_COL;
    if (EDCH(line, off) > 32) { return; }
    while (off && (EDCH(line, off-1) < 33)) { --off; }
}

static void edRdBlk() {
    if (block < 1) { block=0; }
    char *f = blockAddr(block);
    for (int i = 0; i < BLOCK_SZ; i++) { edBuf[i] = f[i]; }
    for (int i = 0; i < BLOCK_SZ; i++) { if (edBuf[i]==0) { edBuf[i] = 32; } }
    isDirty = 0;
    isShow = 1;
}

static void edSvBlk(int force) {
    if (isDirty || force) {
        char *t = blockAddr(block);
        blockIsDirty(block);
        for (int i = 0; i < BLOCK_SZ; i++) { t[i] = edBuf[i]; }
        t[BLOCK_SZ-1] = 0;
    }
    isDirty = 0;
}

static void deleteChar(int toEnd) {
    char *f = &EDCH(line, off);
    char *t = toEnd ? &EDCH(MAX_LINE, MAX_COL) : &EDCH(line, MAX_COL);
    while (f < t) { *(f) = *(f+1); f++; }
    *f = 32;
    DIRTY;
}

static void deleteWord() {
    while (EDCH(line,off) > 32) { deleteChar(0); }
    for (int i=0; i<20; i++) { if (EDCH(line,off)<33) { deleteChar(0); } }
}

static void clrToEOL(int l, int o) {
    while (o<NUM_COLS) { EDCH(l,o)=32; o++; }
    DIRTY;
}

static void deleteLine(int l) {
    for (int r=l; r<MAX_LINE; r++) {
        char *f = &EDCH(r+1, 0);
        char *t = &EDCH(r, 0);
        for (int c=0; c<NUM_COLS; c++) { *(t++) = *(f++); }
    }
    clrToEOL(MAX_LINE, 0);
}

static void yankLine(int l) {
    char *f = &EDCH(l, 0);
    char *t = &yanked[0];
    for (int c=0; c<NUM_COLS; c++) { *(t++) = *(f++); }
    *t = 0;
}

static void putLine(int l) {
    char *f = &yanked[0];
    char *t = &EDCH(l, 0);
    for (int c=0; c<NUM_COLS; c++) { *(t++) = *(f++); }
}

static void insertSpace(int toEnd) {
    if (toEnd) {
        char *f = &EDCH(line,off);
        char *t = &EDCH(MAX_LINE,MAX_COL);
        while (f<t) { *(t) = *(t-1); t--; }
    } else {
        for (int o=MAX_COL; off<o; o--) {
            EDCH(line,o) = EDCH(line, o-1);
        }
    }
    EDCH(line, off)=32;
    DIRTY;
}

static void insertLine(int l) {
    for (int r=MAX_LINE; r>l; r--) {
        char *f = &EDCH(r-1, 0);
        char *t = &EDCH(r, 0);
        for (int c=0; c<NUM_COLS; c++) { *(t++) = *(f++); }
    }
    clrToEOL(l, 0);
}

static void joinLines() {
    if (line == MAX_LINE) { return; }
    gotoEOL();
    int o = off+1;
    char *t = &EDCH(line, o);
    char *f = &EDCH(line+1, 0);
    while ((o++) < NUM_COLS) { *(t++) = *(f++); }
    deleteLine(line+1);
}

static void execLine(int ln) {
    char *x = &EDCH(ln,0);
    char c = x[MAX_COL];
    x[MAX_COL] = 0;
    changeState(INTERP); ttyMode(0);
    toFooter(); emit('\n'); outer(x);
    x[MAX_COL] = c;
}

static void replaceChar(char c, int force, int mov) {
    if (btwi(c,32,126) || (force)) {
        EDCH(line, off)=c;
        DIRTY;
        if (mov) { mv(0, 1); }
    }
}

static void replace1() {
    FG(117); zType("?\x08"); CursorOn();
    int ch = key(); CursorOff();
    replaceChar(ch, 0, 1);
    isShow = 1;
}

static int doInsertReplace(char c) {
    if (c==EOL_LOCAL) {
        mv(1, -NUM_COLS);
        if (edMode == INSERT) { insertLine(line); }
        return 1;
    }
    if (!btwi(c,1,5) && !btwi(c,32,126)) { return 1; }
    if (edMode == INSERT) { insertSpace(0); }
    replaceChar(c, 1, 1);
    return 1;
}

static void edDelX(int c) {
    if (c==0) { c = key(); }
    if (c=='d') { yankLine(line); deleteLine(line); }
    else if (c=='w') { deleteWord(); }
    else if (c=='.') { deleteChar(0); }
    else if (c=='z') { if (0<off) { --off; deleteChar(0); } }
    else if (c=='$') { clrToEOL(line, off); }
}

static int edReadLine(char *buf, int sz) {
    int len = 0;
    CursorOn();
    while (len<(sz-1)) {
        char c = key();
        if (c == EOL_LOCAL) { break; }
        if (c ==  3) { len=0; break; }
        if (c == 27) { len=0; break; }
        if (((c==127) || (c==8)) && (0<len)) { --len; zType("\x08 \x08"); }
        if (btwi(c,32,126)) { buf[len++]=c; emit(c); }
    }
    CursorOff();
    buf[len]=0;
    return len;
}

static void edCommand() {
    char buf[32];
    toCmd(); emit(':'); ClearEOL();
    edReadLine(buf, sizeof(buf));
    toCmd(); ClearEOL();
    if (strEqI(buf,"rl")) { edRdBlk(); }
    else if (buf[0]=='!') { ttyMode(0); outer(&buf[1]); }
    else if (strEqI(buf,"w")) { edSvBlk(0); }
    else if (strEqI(buf,"w!")) { edSvBlk(1); }
    else if (strEqI(buf,"wq")) { edSvBlk(0); edMode=QUIT; }
    else if (strEqI(buf,"q!")) { edMode=QUIT; }
    else if (strEqI(buf,"q")) {
        if (isDirty) { zType("(use 'q!' to quit without saving)"); }
        else { edMode=QUIT; }
    }
}

static void gotoBlock(int blk) {
        edSvBlk(0); setBlock(blk);
        edRdBlk(); line = off = 0;
}

static void toText() {
    char x[NUM_COLS+1];
    sprintf(x,"block-%03d.fth",block);
    cell fh = fileOpen(x, "wb");
    if (fh) {
        outputFp = fh;
        for (int r=0; r<NUM_LINES; r++ ) {
            char *f = &EDCH(r,0);
            for (int r=0; r<NUM_COLS; r++ ) { x[r] = f[r]; }
            x[MAX_COL+1] = 0;
            if (x[MAX_COL]==0) { x[MAX_COL] = 32; }
            for (int r=MAX_COL; 0 <= r; r-- ) {
                if (x[r]==32) { x[r] = 0; }
                else { break; }
            }
            zType(x);
            emit(10);
        }
        outputFp = 0;
        fileClose(fh);
    }
}

static void toBlock() {
    char x[BLOCK_SZ+1];
    sprintf(x,"block-%03d.fth",block);
    cell fh = fileOpen(x, "rb");
    if (fh) {
        for (int i=0; i<BLOCK_SZ; i++ ) { edBuf[i]=32; }
        int n = fileRead(x, BLOCK_SZ, fh); toCmd(); zTypeF("%d chars", n);
        int r=0, c=0;
        fileClose(fh);
        for (int i=0; i<n; i++ ) {
            char ch = x[i];
            if (ch==EOL_LOCAL) { continue; }
            if (ch==9) { ch=32; }
            if (ch==10) {
                while (c < NUM_COLS) { EDCH(r,c)=32; c++; }
                if (r == MAX_LINE) { break; }
                r++; c=0;
            } else { EDCH(r,c)=ch; c++; }
        }
        DIRTY;
    } else { toCmd(); zTypeF("-can't-open [%s]!-", x); }
}

static void doCTL(int c) {
    if (((c == 8) || (c == 127)) && (0 < off)) {      // <backspace>
        mv(0, -1); if (edMode == INSERT) { deleteChar(0); }
        return;
    }
    if (c == EOL_LOCAL) {      // <CR>
        mv(1, -NUM_COLS); if (edMode == INSERT) { insertLine(line); }
        return;
    }
    switch (c) {
        case   1:   doInsertReplace(c);     // COMPLE
        BCASE  2:   doInsertReplace(c);     // DEFINE
        BCASE  3:   doInsertReplace(c);     // INTERP
        BCASE  4:   doInsertReplace(c);     // COMMENT
        BCASE  5:   execLine(line);         // Execute current line
        BCASE  9:   mv(0, 8);               // <tab>
        BCASE 10:   mv(1, 0);               // <ctrl-j>
        BCASE 11:   mv(-1, 0);              // <ctrl-k>
        BCASE 12:   mv(0, 1);               // <ctrl-l>
        BCASE 17:   mv(0, -8);              // <ctrl-q>
        BCASE 24:   edDelX('.');            // <ctrl-x>
        BCASE 20:   edSvBlk(0);             // <ctrl-s>
        BCASE 26:   edDelX('z');            // <ctrl-z>
        BCASE 27:   normalMode();           // <escape>
        BCASE Up:   mv(-1, 0);              // Up
        BCASE Lt:   mv(0, -1);              // Left
        BCASE Rt:   mv(0, 1);               // Right
        BCASE Dn:   mv(1, 0);               // Down
        BCASE Home: mv(0, -NUM_COLS);       // Home
        BCASE End:  gotoEOL();              // End
        BCASE PgUp: gotoBlock(block-1);     // PgUp
        BCASE PgDn: gotoBlock(block+1);     // PgDn
        BCASE Del:  edDelX('.');            // Delete
        BCASE Ins:  toggleInsert();         // Insert
        BCASE CHome: mv(-NUM_LINES, -NUM_COLS);  // <ctrl>-Home
    }
}

static int processEditorChar(int c) {
    CursorOff();
    if (!btwi(c,32,126)) { doCTL(c); return 1; }
    if (btwi(edMode,INSERT,REPLACE)) { return doInsertReplace((char)c); }

    switch (c) {
        BCASE ' ': mv(0, 1);
        BCASE '#': CLS(); isShow=1;
        BCASE '$': gotoEOL();
        BCASE '_': mv(0,-NUM_COLS);
        BCASE '1': replaceChar(1,1,0); // COMPILE
        BCASE '2': replaceChar(2,1,0); // DEFINE
        BCASE '3': replaceChar(3,1,0); // INTERP
        BCASE '4': replaceChar(4,1,0); // COMMENT
        BCASE '+': gotoBlock(block+1); // Next block
        BCASE '-': gotoBlock(block-1); // Prev block
        BCASE ':': edCommand();
        BCASE 'a': mv(0, 1); insertMode();
        BCASE 'A': gotoEOL(); insertMode();
        BCASE 'b': insertSpace(0);
        BCASE 'B': insertSpace(1);
        BCASE 'c': edDelX('.'); insertMode();
        BCASE 'C': edDelX('$'); insertMode();
        BCASE 'd': edDelX(0);
        BCASE 'D': edDelX('$');
        BCASE 'g': mv(-NUM_LINES,-NUM_COLS);
        BCASE 'G': mv(NUM_LINES,-NUM_COLS);
        BCASE 'h': mv(0,-1);
        BCASE 'H': gotoBlock((block & 0x01) ? block+1 : block-1);
        BCASE 'i': insertMode();
        BCASE 'I': mv(0, -NUM_COLS); insertMode();
        BCASE 'j': mv(1, 0);
        BCASE 'J': joinLines();
        BCASE 'k': mv(-1,0);
        BCASE 'l': mv(0, 1);
        BCASE 'n': replaceChar(10,1,0);
        BCASE 'o': mv(1, -NUM_COLS); insertLine(line); insertMode();
        BCASE 'O': mv(0, -NUM_COLS); insertLine(line); insertMode();
        BCASE 'p': mv(1,-NUM_COLS); insertLine(line); putLine(line);
        BCASE 'P': mv(0,-NUM_COLS); insertLine(line); putLine(line);
        BCASE 'q': mv(0,8);
        BCASE 'Q': mv(0,-8);
        BCASE 'r': replace1();
        BCASE 'R': replaceMode();
        BCASE 't': toText();
        BCASE 'T': toBlock();
        BCASE 'w': moveWord(1);
        BCASE 'W': moveWord(0);
        BCASE 'x': deleteChar(0);
        BCASE 'X': deleteChar(1);
        BCASE 'Y': yankLine(line);
        BCASE 'z': edDelX('z');
    }
    return 1;
}

static void showFooter() {
    const char *x[3] = { "-normal-","-insert-","-replace-" };
    char ch = EDCH(line,off);
    toFooter(); FG(255);
    zTypeF("Block# %03d%s", block, isDirty ? " *" : "");
    if (edMode != NORMAL) { FG(203); }
    zTypeF(" %s", x[edMode-1]);
    if (edMode != NORMAL) { FG(255); }
    zTypeF(" (%d:%d - #%d/$%02x)", line+1, off+1, ch, ch);
    ClearEOL();
}

static void showEditor() {
    if (!isShow) { return; }
    FG(40); GotoXY(1,1);
    for (int i=-2; i<NUM_COLS; i++) { emit('-'); } zType("\r\n");
    for (int r=0; r<NUM_LINES; r++) {
        zType("|"); showState(0);
        for (int c=0; c<NUM_COLS; c++) {
            char ch = EDCH(r,c);
            if (btwi(ch,1,4)) { showState(ch); }
            emit(MAX(ch,32));
        }
        FG(40); zType("|\r\n"); 
    }
    for (int i=-2; i<NUM_COLS; i++) { emit('-'); }
    isShow = 0;
}

void editBlock(cell blk) {
    setBlock(blk);
    line = off = 0;
    CLS();
    edRdBlk();
    isShow = 1;
    normalMode();
    while (edMode != QUIT) {
        showEditor();
        showFooter();
        showCursor();
        curLine = &EDCH(line,0);
        edBuf[BLOCK_SZ-1]=0;
        processEditorChar(edKey());
    }
    toCmd();
    CursorOn(); FG(255);
}

#endif //  EDITOR
