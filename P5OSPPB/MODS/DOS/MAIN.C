#include "../include/p5.h"

#define CMD_COUNT 4 


//Function declarations
void usrClear(void);
void consVer(void);
void chPrompt(void);
void swap(void);

unsigned char prompt[50];

//Typedefs
typedef void (*sys_command)(void);


//Variable declarations
char* cmdWord[CMD_COUNT] = {
    "CLR",
    "VER",
    "PROMPT",
    "SWITCH"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&usrClear,
    (sys_command)&consVer,
    (sys_command)&chPrompt,
    (sys_command)&swap
};

char inbuf[50];


void parse(char* cmdbuf) {

    int i, found;
    
    found = 0;
    for(i = 0; i < CMD_COUNT; i++) {
        
        if(strcmp(cmdWord[i], cmdbuf)) {
        
            found = 1;
            cmdFunc[i]();
            break;
        }
    }
    
    if(!found) {
        
        prints("Unknown command ");
        prints(cmdbuf);
        prints("\n");
    }
}

void main(void) {
    
    prompt = "DOS"
    
    prints("Entered console\n");
    while(1) {
        prints(&prompt[0]);
        prints("> ");
        scans(50, inbuf);
        parse(inbuf);
    }    
}


int strcmp(char* s1, char* s2) {
    
    int i;
    
    for(i = 0; s1[i] && s2[i]; i++)
        if(s1[i] != s2[i])
            return 0;
    
    if(s1[i] != s2[i])
        return 0;
        
    return 1;
}


void usrClear(void) {

    clearScreen();
}


void consVer(void) {

    prints("P5 usermode console build 1\n");
    prints("P5 build [need fmt print and P5 build number hook]\n");
}


void chPrompt() {

    prints("  New console prompt: ");
    scans(50, &prompt[0]);
}


void swap(void) {

    nextProc();
}
