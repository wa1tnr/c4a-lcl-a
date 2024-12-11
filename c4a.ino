// Support for development boards

#include "c4a.h"

#define mySerial Serial // Teensy and Pico

#ifdef mySerial
    void serialInit() {
        mySerial.begin(9600);
        while (!mySerial);
        mySerial.println();
        mySerial.println();
        mySerial.print("    c4a local tnr version: ");
        mySerial.print(__DATE__);
        mySerial.write(' ');
        mySerial.println(__TIME__);
        mySerial.write(' ');
        mySerial.println("    tannis feldspar delta  __");
    }
    void emit(char c) { mySerial.print(c); }
    void zType(const char *s) { mySerial.print(s); }
    int qKey() { return mySerial.available(); }
    int key() { 
        while (!qKey()) {}
        return mySerial.read();
    }
#else
    void serialInit() { }
    void emit(char c) {}
    void zType(char *s) {}
    int qKey() { return 0; }
    int key() { return 0; }
#endif

cell timer() { return micros(); }
void ttyMode(int isRaw) {}

// Cells are always 32-bit on dev boards (no 64-bit)
#define S1(x, y) (*(byte*)(x)=((y)&0xFF))
void store32(cell l, cell v) {
    if (((cell)l & 0x03) == 0) { *(cell*)l = v; }
    else {
        S1(l,v); S1(l+1,v>>8); S1(l+2,v>>16); S1(l+3,v>>24);
    }
}

void store16(cell l, cell v) {
    if (((cell)l & 0x03) == 0) { *(short*)l = (short)v; }
    else {
        S1(l,v); S1(l+1,v>>8);;
    }
}

#define F1(x, y) (*(byte*)(x)<<y)
cell fetch32(cell l) {
    if (((cell)l & 0x03) == 0) { return *(cell*)l; }
    return F1(l,0) | F1(l+1,8) | F1(l+2,16) | F1(l+3,24);
}

cell fetch16(cell l) {
    if (((cell)l & 0x03) == 0) { return *(short*)l; }
    return F1(l,0) | F1(l+1,8);
}

char *in, tib[128];
void setup() {
  delay(900);
  serialInit();
  c4Init();
  outer(" .\" %nc4 - version \" .version cr");
  zType("Hello.");
  ok();
  in = (char*)0;
}

void idle() {
  // Fill this in as desired
}

void loop() {
  if (qKey() == 0) { idle(); return; }
  int c = key();
  if (!in) {
      in = tib;
      for (int i=0; i<128; i++) { tib[i] = 0; }
  }

  if (c==9) { c = 32; }
  /* if (c==13) { */
  if (c==EOL_LOCAL) {
      *(in) = 0;
      emit(32); outer(tib);
      ok();
      in = 0;
  } else if ((c==8) || (c==127)) {
      if ((--in) < tib) { in = tib; }
      else { emit(8); emit(32); emit(8); }
  } else {
      if (btwi(c,32,126)) { *(in++) = c; emit(c); }
  }
}
