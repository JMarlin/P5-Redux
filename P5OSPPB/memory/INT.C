
idtPtr* idtBase = (unsigned int*)0x201000;
idtEntry* idtEntries = (unsigned int*)0x201030;  

void initIDT() {
       
                                

}

void installInterrupt(unsigned char number, intHandler* handler) {
        
        idtEntries[number].offset_1 = (unsigned short)(((unsigned int)handler) & 0xFFFF);
        idtEntries[

}
