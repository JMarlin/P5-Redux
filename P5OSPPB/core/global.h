#ifndef GLOBAL_H
#define GLOBAL_H

//#define PRINT_DEBUG

#ifndef PRINT_DEBUG
#define DEBUG(x)
#define DEBUG_HB(x)
#define DEBUG_HW(x)
#define DEBUG_HD(x)
#define DEBUG_PCHAR(x)
#else
#define DEBUG(x)    prints(x)
#define DEBUG_HB(x) printHexByte(x)
#define DEBUG_HW(x) printHexWord(x)
#define DEBUG_HD(x) printHexDword(x)
#define DEBUG_PCHAR(x) pchar(x)
#endif //PRINT_DEBUG

#endif //GLOBAL_H
