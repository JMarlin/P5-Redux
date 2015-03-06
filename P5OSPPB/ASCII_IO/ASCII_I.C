#include "ascii_i.h"
#include "ascii_o.h"
#include "keyboard.h"


unsigned char keyTable[132];

void setupKeyTable() {
    int i;
    
    for(i=0;i<132;i++)
        keyTable[i] = 0;
        
    keyTable[0xE] = '`';
    keyTable[0x15] = 'Q';
    keyTable[0x16] = '1';
    keyTable[0x1A] = 'Z';
    keyTable[0x1B] = 'S';
    keyTable[0x1C] = 'A';
    keyTable[0x1D] = 'W';
    keyTable[0x1E] = '2';
    keyTable[0x21] = 'C';
    keyTable[0x22] = 'X';
    keyTable[0x23] = 'D';
    keyTable[0x24] = 'E';
    keyTable[0x25] = '4';
    keyTable[0x26] = '3';
    keyTable[0x29] = ' ';
    keyTable[0x2A] = 'V';
    keyTable[0x2B] = 'F';
    keyTable[0x2C] = 'T';
    keyTable[0x2D] = 'R';
    keyTable[0x2E] = '5';
    keyTable[0x31] = 'N';
    keyTable[0x32] = 'B';
    keyTable[0x33] = 'H';
    keyTable[0x34] = 'G';
    keyTable[0x35] = 'Y';
    keyTable[0x36] = '6';
    keyTable[0x3A] = 'M';
    keyTable[0x3B] = 'J';
    keyTable[0x3C] = 'U';
    keyTable[0x3D] = '7';
    keyTable[0x3E] = '8';
    keyTable[0x41] = ',';
    keyTable[0x42] = 'K';
    keyTable[0x43] = 'I';
    keyTable[0x44] = 'O';
    keyTable[0x45] = '0';
    keyTable[0x46] = '9';
    keyTable[0x49] = '.';
    keyTable[0x4A] = '/';
    keyTable[0x4B] = 'L';
    keyTable[0x4C] = ';';
    keyTable[0x4D] = 'P';
    keyTable[0x4E] = '-';
    keyTable[0x52] = '\'';
    keyTable[0x54] = '[';
    keyTable[0x55] = '=';
    keyTable[0x5A] = '\n';
    keyTable[0x5B] = ']';
}


void setupKeyTable_phys() {
    int i;
    
    for(i=0;i<132;i++)
        keyTable[i] = 0;
        
    keyTable[0x1E] = 'A';
    keyTable[0x30] = 'B';
    keyTable[0x2E] = 'C';
    keyTable[0x20] = 'D';
    keyTable[0x12] = 'E';
    keyTable[0x21] = 'F';
    keyTable[0x22] = 'G';
    keyTable[0x23] = 'H';
    keyTable[0x17] = 'I';
    keyTable[0x24] = 'J';
    keyTable[0x25] = 'K';
    keyTable[0x26] = 'L';
    keyTable[0x32] = 'M';
    keyTable[0x31] = 'N';
    keyTable[0x18] = 'O';
    keyTable[0x19] = 'P';
    keyTable[0x10] = 'Q';
    keyTable[0x13] = 'R';
    keyTable[0x1f] = 'S';
    keyTable[0x14] = 'T';
    keyTable[0x16] = 'U';
    keyTable[0x2f] = 'V';
    keyTable[0x11] = 'W';
    keyTable[0x2D] = 'X';
    keyTable[0x15] = 'Y';
    keyTable[0x2c] = 'Z';
    keyTable[0x0b] = '0';
    keyTable[0x02] = '1';
    keyTable[0x03] = '2';
    keyTable[0x04] = '3';
    keyTable[0x05] = '4';
    keyTable[0x06] = '5';
    keyTable[0x07] = '6';
    keyTable[0x08] = '7';
    keyTable[0x09] = '8';
    keyTable[0x0A] = '9';
    keyTable[0x29] = '`';
    keyTable[0x0c] = '-';
    keyTable[0x0d] = '=';
    keyTable[0x2b] = '\\';
    keyTable[0x1c] = '\n';
    keyTable[0x1a] = '[';
    keyTable[0x1b] = ']';
    keyTable[0x27] = ';';
    keyTable[0x28] = '\'';
    keyTable[0x33] = ',';
    keyTable[0x34] = '.';
    keyTable[0x35] = '/';
}


void scans(unsigned int length, char* outstr) {
    
    unsigned char temp_char;
    int index = 0;
  
    for(index = 0 ; index < length-1 ; ) {    
        temp_char = getch();

        if(temp_char != 0) {       
            outstr[index] = temp_char;
            pchar(outstr[index]);
    
            if(outstr[index] == '\n') {
                outstr[index] = 0;
                break;
            }

            index++;
            
            if(index == length-1)
                pchar('\n');    
        }
    }
    
    outstr[index+1] = 0;
}


unsigned char getch(void) {
        
    unsigned char tempData;
    
    //Don't block if there's nothing in the buffer
    if(!(keyboard_getStatus() & SR_OBSTAT)) 
        return 0;
    
    tempData = inb(KBC_DREG);
    
    if(tempData == 0xF0) {
        keyboard_getData();
        return 0;
    }

    if(tempData < 132) {
        return keyTable[tempData];
    } else {
        return 0;
    }

    //This should make realines cycle forever waiting for input
    return 0;
}


int strcmp(char* in1, char* in2) {

    int index = 0;

    while(in1[index] != 0 && in2[index] != 0) {
        
        if(in1[index] != in2[index])
            return 0;
        
        index++;    
    }
    
    return 1;
}

int strcmpci(char* in1, char* in2) {
    
    int index = 0;
    
    while(in1[index] != 0 && in2[index] != 0) {
    
        if(in1[index] > 64 && in1[index] < 91)
            in1[index] += 32;
    
        if(in2[index] > 64 && in2[index] < 91)
            in2[index] += 32;              
       
        if(in1[index] != in2[index])
            return 0;
       
        index++;
    }
    
    return 1;
}

int slen(char* ins) {

    int index = 0;

    while(ins[index] != 0) {
        index++;        
    }
    
    return index;
}
