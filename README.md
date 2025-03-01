# c4a: a 32-bit Forth system for PCs and Arduino, inspired by ColorForth and Tachyon

## ColorForth's influence on C4A
- C4A supports control characters in the whitespace that change the state.<br/>
- C4A has 4 states: INTERPRET, COMPILE, DEFINE, AND COMMENT,<br/>
- C4A also supports the standard state-change words.<br/>

| Ascii | Word  | State | Description|
|:--    |:--    |:--    |:-- |
|  $01  |  ]    |   1   | Compile |
|  $02  |  :    |   2   | Define |
|  $03  |  [    |   3   | Interpret/execute/immediate |
|  $04  |       |   4   | Comment |
|       |  (    |   4   | Comment, save current state |
|       |  )    |       | End comment, restores saved state |

**NOTE**: In the DEFINE state, C4A changes the state to COMPILE after adding the next word.<br/>
**NOTE**: Unlike ColorForth, ';' compiles EXIT and then changes the state to INTERPRET.<br/>

## Tachyon's influence on C4A
- In C4A, a program is a sequence of WORD-CODEs. <br/>
- A WORD-CODE is a 32-bit unsigned number (i.e. - a DWORD). <br/>
- Primitives are assigned numbers sequentially from 0 to [BYE]. <br/>
- If a WORD-CODE is less than or equal to [BYE], it is a primitive. <br/>
- If the top 3 bits are set, it is a 29-bit unsigned literal, 0-$1FFFFFFF. <br/>
- If it is between [BYE], and $E0000000, it is the code address of a word to execute. <br/>

## CELLs in C4A
- A **CELL** in C4A is 32-bits, the same size as a **WORD-CODE**.
- Since development boards don't support 64-bits, neither does c4a.
- For PCs, only 32-bit platforms are supported.
- For 64-bit systems, see c4: https://github.com/CCurl/c4).

## Building C4A
 
### PCs - Windows and Linux, and probably others
- Windows: there is a c4a.sln file for Visual Studio
  - only the x86 target is supported
- Linux: there is a makefile
  - only the 32-bit configuration (-m32) is supported
- Others:
  - c4a is simple enough that it should be easy to migrate it to any platform

### Development boards via the Arduino IDE:
- I use the Arduino IDE v2.0
- There is a c3.ino file
- File `c4a.h` controls parameters for the target board
- Edit the section where isBOARD is defined to set the configuration for the board
- Use `#define FILE_NONE` to disable support for blocks and LittleFS
- For the RPI Pico:
  - Use the arduino-pico from earlephilhower (https://github.com/earlephilhower/arduino-pico)
  - The version must be 4.2.0 or later. Versions older than 4.0.0 do not support boards using
    the RP2350 microcontroller.
  - Use `#define FILE_PICO` to include support for LittleFS
- For the Teensy-4.x:
  - Use `#define FILE_TEENSY` to include support for LittleFS

## C4A memory areas
C4A provides a single memory area. See 'mem-sz' (MEM_SZ in c4a.h) for its size.
- It is broken into 3 areas: CODE, VARS, and DICT.
- The CODE area is an aray of WORD-CODEs starting at the beginning of the memory.
  - `here` is an offset into the CODE area.
  - The size of the CODE area is `code-sz`. See 'code-sz' (CODE_SZ in c4a.h).
  - **NOTE**: Use `wc@` and `wc!` to get and set WORD-CODE values in the code area.
  - **NOTE**: CODE slots 0-25 (`0 wc@ .. 25 wc@`) are reserved for C4A system values.
  - **NOTE**: CODE slots 26-[BYE] (`26 wc@` .. `[BYE] wc@`) are unused by C4A.
  - **NOTE**: So c4a provides space for about 75 'free' variables.
  - **NOTE**: These are free for the user/application to use as desired.
- The VARS area is defined to begin at address `code-sz wc-sz * memory +`.
  - `vhere` is the absolute address of the first free byte the VARS area.
- The DICT is at the end of the memory. 'last' grows toward the beginning of the memory.
  - `last` is an offset into the memory area.
- Use `->memory` to turn an offset into an address.

| WORD    | STACK | DESCRIPTION |
|:--      |:--    |:-- |
| memory  | (--A) | A: starting address of the C4A memory |
| mem-sz  | (--N) | N: size in BYTEs of the C4A memory |
| code-sz | (--N) | N: number of in WORD-CODE slots in the code area |
| dstk-sz | (--N) | N: size in CELLs of the DATA and RETURN stacks |
| tstk-sz | (--N) | N: size in CELLs of the A and T stacks |
| wc-sz   | (--N) | N: size in BYTEs of a WORD-CODE |
| de-sz   | (--N) | N: size in BYTEs of a dictionary entry |
| (dsp)   | (--N) | N: CODE slot for the data stack pointer |
| (rsp)   | (--N) | N: CODE slot for the return stack pointer |
| (lsp)   | (--N) | N: CODE slot for the loop stack pointer |
| (tsp)   | (--N) | N: CODE slot for the T stack pointer |
| (asp)   | (--N) | N: CODE slot for the A stack pointer |
| (here)  | (--N) | N: CODE slot for the HERE variable |
| (last)  | (--N) | N: CODE slot for the LAST variable |
| base    | (--N) | N: CODE slot for the BASE variable |
| state   | (--N) | N: CODE slot for the STATE variable |

## C4A Strings
Strings in C4A are NULL-terminated with no count byte.<br/>

## Format specifiers in `ftype` and `."`
Similar to the printf() function in C, C4A supports formatted output using '%'. <br/>
For example `: ascii dup dup dup ." char %c, decimal #%d, binary: %%%b, hex: $%x%n" ;`.

| Format | Stack | Description |
|:--     |:--    |:-- |
| %b     | (N--) | Print TOS in base 2. |
| %c     | (N--) | EMIT TOS. |
| %d     | (N--) | Print TOS in base 10. |
| %e     | (--)  | EMIT `escape` (#27). |
| %i     | (N--) | Print TOS in the current base. |
| %n     | (--)  | Print CR/LF (13/10). |
| %q     | (--)  | EMIT `"` (#34). |
| %s     | (A--) | Print TOS as a string (formatted). |
| %S     | (A--) | Print TOS as a string (unformatted). |
| %x     | (N--) | Print TOS in base 16. |
| %[x]   | (--)  | EMIT [x]. |

## The A stack
C4A includes an A stack. <br/>
This is somewhat similar to MachineForth's operations for 'a', but in C4A, it is a stack.<br/>
The size of the A stack is configurable (see `tstk-sz`).<br/>

| WORD  | STACK | DESCRIPTION |
|:--    |:--    |:-- |
| `>a`  | (N--) | Push N onto the A stack. |
| `a!`  | (N--) | Set A-TOS to N. |
| `a@`  | (--N) | N: copy of A-TOS. |
| `a@+` | (--N) | N: copy of A-TOS, then increment A-TOS. |
| `a@-` | (--N) | N: copy of A-TOS, then decrement A-TOS. |
| `a>`  | (--N) | Pop N from the A stack. |
| adrop | (--)  | Drop A-TOS |

## The T Stack
C4A includes a T stack, with tge same operations as the A stack. <br/>
Note that there are also additional words for the return stack. <br/>

| WORD  | STACK | DESCRIPTION |
|:--    |:--    |:-- |
| `>t`  | (N--) | Push N onto the T stack. |
| `t!`  | (N--) | Set T-TOS to N. |
| `t@`  | (--N) | N: copy of T-TOS. |
| `t@+` | (--N) | N: copy of T-TOS, then increment T-TOS. |
| `t@-` | (--N) | N: copy of T-TOS, then decrement T-TOS. |
| `t>`  | (--N) | Pop N from the T stack. |
| tdrop | (--)  | Drop T-TOS |

## C4A WORD-CODE primitives
Stack effect notation conventions:

| TERM     | DESCRIPTION |
|:--       |:-- |
| SZ/NM/MD | String, uncounted, NULL terminated |
| A        | Address |
| C        | Number, 8-bits |
| WC       | WORD-CODE, 32-bits |
| N/X/Y    | Number, CELL sized |
| F        | Flag: 0 mean0 false, <>0 means true |
| R        | Register number |
| FH       | File handle: 0 means no file |
| I        | For loop index counter |

The primitives:

| WORD        | STACK        | DESCRIPTION |
|:--          |:--           |:-- |
| (lit)       | (--WC)       | WC: WORD-CODE for LIT primitive |
| (jmp)       | (--WC)       | WC: WORD-CODE for JMP primitive |
| (jmpz)      | (--WC)       | WC: WORD-CODE for JMPZ primitive |
| (jmpnz)     | (--WC)       | WC: WORD-CODE for JMPNZ primitive |
| (njmpz)     | (--WC)       | WC: WORD-CODE for NJMPZ primitive |
| (njmpnz)    | (--WC)       | WC: WORD-CODE for NJMPNZ primitive |
| (exit)      | (--WC)       | WC: WORD-CODE for EXIT primitive |
| exit        | (--)         | EXIT word |
| dup         | (X--X X)     | Duplicate TOS (Top-Of-Stack) |
| swap        | (X Y--Y X)   | Swap TOS and NOS (Next-On-Stack) |
| drop        | (N--)        | Drop TOS |
| over        | (N X--N X N) | Push NOS |
| @           | (A--N)       | N: the CELL at absolute address A |
| c@          | (A--C)       | C: the CHAR at absolute address A |
| w@          | (A--W)       | W: the WORD at absolute address A |
| wc@         | (N--WC)      | WC: the WORD-CODE in CODE slot N |
| !           | (N A--)      | Store CELL N to absolute address A |
| c!          | (C A--)      | Store CHAR C to absolute address A |
| w!          | (W A--)      | Store WORD W to absolute address A |
| wc!         | (WC N--)     | Store WORD-CODE WC to CODE slot N |
| +           | (X Y--N)     | N: X + Y |
| -           | (X Y--N)     | N: X - Y |
| *           | (X Y--N)     | N: X * Y |
| /           | (X Y--N)     | N: X / Y (integer division) |
| /mod        | (X Y--M Q)   | M: X modulo Y, Q: quotient of X / Y |
| 1+          | (X--Y)       | Increment TOS |
| 1-          | (X--Y)       | Decrement TOS |
| <           | (X Y--F)     | F: 1 if (X < Y), else 0 |
| =           | (X Y--F)     | F: 1 if (X = Y), else 0 |
| >           | (X Y--F)     | F: 1 if (X > Y), else 0 |
| 0=          | (N--F)       | F: 1 if (N=0), else 0 |
| and         | (X Y--N)     | N: X AND Y |
| or          | (X Y--N)     | N: X OR  Y |
| xor         | (X Y--N)     | N: X XOR Y |
| com         | (X--Y)       | Y: X with all bits flipped (complement) |
| for         | (N--)        | Begin FOR loop with bounds 0 and N-1. |
| i           | (--I)        | N: Current FOR loop index. |
| next        | (--)         | Increment I. If I >= N, exit, else start loop again. |
| unloop      | (--)         | Unwind the loop stack. **NOTE:** does NOT exit the loop. |
| >r          | (N--)        | Push N onto the return stack |
| r!          | (N--)        | Set R-TOS to N |
| r@          | (--N)        | N: copy of R-TOS |
| r@+         | (--N)        | N: copy of R-TOS, then increment it |
| r@-         | (--N)        | N: copy of R-TOS, then decrement it |
| r>          | (--N)        | Pop N from the return stack |
| rdrop       | (--)         | Drop R-TOS |
| >t          | (N--)        | Push N onto the T stack |
| t!          | (N--)        | Set T-TOS to N |
| t@          | (--N)        | N: copy of T-TOS |
| t@+         | (--N)        | N: copy of T-TOS, then increment T-TOS |
| t@-         | (--N)        | N: copy of T-TOS, then decrement T-TOS |
| t>          | (--N)        | Pop N from the T stack |
| tdrop       | (--)         | Drop T-TOS |
| >a          | (N--)        | Push N onto the A stack |
| a!          | (N--)        | Set A-TOS to N |
| a@          | (--N)        | N: copy of A-TOS |
| a@+         | (--N)        | N: copy of A-TOS, then increment A-TOS |
| a@-         | (--N)        | N: copy of A-TOS, then decrement A-TOS |
| a>          | (--N)        | Pop N from the A stack |
| adrop       | (--)         | Drop A-TOS |
| emit        | (C--)        | Output char C |
| ;           | (--)         | Compile EXIT, set STATE=INTERPRET |
| lit,        | (N--)        | Compile a push of number N |
| next-wd     | (--L)        | L: length of the next word from the input stream |
| immediate   | (--)         | Mark the last created word as IMMEDIATE |
| inline      | (--)         | Mark the last created word as INLINE |
| outer       | (S--)        | Send string S to the C4A outer interpreter |
| addword     | (--)         | Add the next word to the dictionary |
| timer       | (--N)        | N: Current time |
| see X       | (--)         | Output the definition of word X |
| ztype       | (S--)        | Print string at S (unformatted) |
| ftype       | (S--)        | Print string at S (formatted) |
| s-cpy       | (D S--D)     | Copy string S to D |
| s-eq        | (D S--F)     | F: 1 if string S is equal to D (case sensitive) |
| s-eqi       | (D S--F)     | F: 1 if string S is equal to D (NOT case sensitive) |
| s-len       | (S--N)       | N: Length of string S |
| z"          | (--)         | -COMPILE: Create string S to next `"` |
|             | (--S)        | -RUN: push address S of string |
| ."          | (--)         | -COMPILE: execute `z"`, compile `ftype` |
|             | (--)         | -RUN: `ftype` on string |
| find        | (--XT A)     | XT: Execution Token, A: Dict Entry address (0 0 if not found) |
| loaded?     | (XT A--)     | Stops current load if A <> 0 (see `find`) |
| fopen       | (NM MD--FH)  | NM: File Name, MD: Mode, FH: File Handle (0 if error/not found) |
| fclose      | (FH--)       | FH: File Handle to close |
| fdelete     | (NM--)       | NM: File Name to delete |
| fread       | (A N FH--X)  | A: Buffer, N: Size, FH: File Handle, X: num chars read |
| fwrite      | (A N FH--X)  | A: Buffer, N: Size, FH: File Handle, X: num chars written |
| fseek       | (N FH--X)    | N: Size, FH: File Handle, X: return from func |
| fsize       | (FH--N)      | FH: File Handle, N: file size |
| fpos        | (FH--N)      | FH: File Handle, N: current file read/write position |
| load        | (N--)        | N: Block number to load |
| load-next   | (N--)        | Close the current block and load block N next |
| blocks      | (--)         | Dump block cache |
| block-addr  | (N--A)       | N: Block number, A: Address in cache |
| flush       | (F--)        | F: True => clear cache |
| flush-block | (N F--)      | N: Block number, F: True => clear cache entry |
| edit        | (N--)        | N: Block number to edit |
| system      | (S--)        | PC ONLY: S: String to send to `system()` |
| bye         | (--)         | PC ONLY: Exit C4A |

## C4A default words
Default words are defined in function `sys_load()` in file sys-load.cpp.<br/>
For details, or to add or change the default words, modify that function.
