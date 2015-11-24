#include <stdio.h>
#include <SDL.h>
#include "../../mods/include/wyg.h"
#include "../../mods/include/p5.h"
#include "../../mods/include/gfx.h"

#undef main

//This way we get access to the entry point of WYG
extern void WYG_main(void);
extern unsigned char font_array[];
extern SDL_Renderer* renderer;

typedef struct window {
    unsigned char flags;
    unsigned int handle;
    unsigned int pid;
    bitmap* context;
    struct window* next_sibling;
    struct window* parent; 
    struct window* first_child;
    unsigned int w;
    unsigned int h;
    unsigned int x;
    unsigned int y;
    unsigned char needs_redraw;
    unsigned char* title;
    unsigned char frame_needs_redraw;
} window;

int main(int argc, char** argv) {
	
	WYG_main();
	
	return 0;
}

#define CMD_COUNT 7

//Function declarations
int usrClear(void);
int consVer(void);
int usrExit(void);
int makeChild(void);
int closeChild(void);
int focusCmd(void);
int moveChild(void);
void cmd_pchar(unsigned char c);
void cmd_prints(unsigned char* s);
void cmd_clear();
void cmd_init(unsigned int win);
void cmd_getCursor(unsigned char *x, unsigned char *y);
void cmd_putCursor(unsigned char x, unsigned char y);
void cmd_printHexByte(unsigned char byte);
void cmd_printHexWord(unsigned short wd);
void cmd_printHexDword(unsigned int dword);
void cmd_printDecimal(unsigned int dword);
void cmd_scans(int c, char* b);

//Typedefs
typedef int (*sys_command)(void);

//Variable declarations
char* cmdWord[CMD_COUNT] = {
    "CLR",
    "VER",
    "EXIT",
    "WIN",
    "CLOSE",
    "FOCUS",
    "MOV"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&usrClear,
    (sys_command)&consVer,
    (sys_command)&usrExit,
    (sys_command)&makeChild,
    (sys_command)&closeChild,
    (sys_command)&focusCmd,
    (sys_command)&moveChild
};

char inbuf[50];

int parse(char* cmdbuf) {

    int i, found;

    found = 0;
    for(i = 0; i < CMD_COUNT; i++) {

        if(!strcmp(cmdWord[i], cmdbuf)) {

            return cmdFunc[i]();
        }
    }

    cmd_prints("Unknown command ");
    cmd_prints(cmdbuf);
    cmd_prints("\n");
    
    return 0;
}

unsigned int window_a = 0, window_b = 0;

int focusCmd() {
    
    raiseHandle(window_a);
    return 0;
}

unsigned int winx, winy;

int moveChild() {
    
    if(!window_b) {
        
        cmd_prints("No window\n");
        return;
    }   
    
    winx += 20;
    winy += 20;
    moveWindow(window_b, winx, winy);
    
    return 0;
}

int makeChild() {
    
    bitmap* ctx_b;
    int x, y;
    unsigned int tile_width = 4;
    unsigned int tile_height = 4;
    unsigned int tile_data[] = {
        0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,
        0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000
    };
    
    winx = 100;
    winy = 20;
    
    if(window_b) {
        
        cmd_prints("Raising window\n");
        raiseHandle(window_b);
        return 0;
    }    
    
    cmd_prints("Creating window\n");
    
    window_b = newWindow(400, 400, WIN_FIXEDSIZE, 0);
    
    //Set up their titles
    setWindowTitle(window_b, "Window B");
    
    //Install them into the root window
    installWindow(window_b, ROOT_WINDOW);
    
    //Paint a pretty picture into window A
    ctx_b = getWindowContext(window_b);
    
    //This SHOULD tile the tile image across the window
    for(x = 0; x < 400; x++)
        for(y = 0; y < 400; y++)
            ctx_b->data[y*(400) + x] = tile_data[(y%tile_height)*tile_width + (x%tile_width)];
    
    //Make them prettily cascade
    moveWindow(window_b, 100, 20);
    
    //Make them visible
    markHandleVisible(window_b);
    
    return 0;
}

int closeChild() {
    
    if(window_b) {
     
        cmd_prints("Destroying window\n");   
        destroyHandle(window_b);
        window_b = 0;
        return;
    }
    
    cmd_prints("Window doesn't exist\n");
    
    return 0;
}

void makeWindows() {
    
    unsigned short w, h;
        
    //Make two windows
    window* root_window = getWindowByHandle(ROOT_WINDOW);
    w = root_window->w;
    h = root_window->h;
    
    window_a = newWindow(w - 108, h - 132, WIN_FIXEDSIZE, 0);
    
    //Set up their titles
    setWindowTitle(window_a, "PTerm");
    
    //Install them into the root window
    installWindow(window_a, ROOT_WINDOW);
        
    //Make them prettily cascade
    moveWindow(window_a, 54, 66);
    
    //Make them visible
    markHandleVisible(window_a);
        
    //Set up the console commands
    cmd_init(window_a);
    
    while(1) {

        cmd_prints("::");
        cmd_scans(50, inbuf);
        
        //If the command function returns 1 it signals that we need to exit
        if(parse(inbuf))
            break;
    }
}

int usrClear(void) {

    cmd_clear();
    return 0;
}


int consVer(void) {

    cmd_prints("P5 usermode console build 1\n");
    cmd_prints("P5 build [need fmt print and P5 build number hook]\n");
    return 0;
}


int usrExit(void) {

    destroyHandle(window_a);
    return 1;
}

//Wrapper for setting the blit mask for the window bitmap to a specific region before requesting redraw
void repaintAll(unsigned int handle, bitmap* h_bmp) {
    
    //Set the blitting rect 
    h_bmp->top = 0;
    h_bmp->left = 0;
    h_bmp->bottom = h_bmp->height;
    h_bmp->right = h_bmp->width;   
    
    //Redraw 
    drawHandle(handle);
}

//Wrapper for setting the blit mask for the window bitmap to a specific region before requesting redraw
void repaintRegion(unsigned int handle, bitmap* h_bmp, unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
    
    //Set the blitting rect 
    h_bmp->top = y;
    h_bmp->left = x;
    h_bmp->bottom = y + h;
    h_bmp->right = x + w;   
    
    //Redraw 
    drawHandle(handle); 
}

bitmap* cmd_bmp;
unsigned int cmd_window;
unsigned char cmd_x;
unsigned char cmd_y;
unsigned short cmd_bx, cmd_by; 
int cmd_width;
int cmd_height;
int cmd_max_chars;
int cmd_max_lines;

void drawCharacter(bitmap* b, char c, int x, int y, unsigned int color) {
    
    int j, i;
    unsigned char line;
    c &= 0x7F; //Reduce to base ASCII set

    for(i = 0; i < 12; i++) {

        line = font_array[i * 128 + c];
        for(j = 0; j < 8; j++) {

            if(line & 0x80) b->data[(y + i)*b->width + (x + j)] = color;
            line = line << 1;
        }
    }
    
    repaintRegion(cmd_window, cmd_bmp, x, y, 8, 12);
}


void drawCharacterBold(bitmap* b, char c, int x, int y, unsigned int color) {

    drawCharacter(b, c, x, y, color);
    drawCharacter(b, c, x+1, y, color);
    drawCharacter(b, c, x, y+1, color);
    drawCharacter(b, c, x+1, y+1, color);
}


void drawString(bitmap* b, char* str, int x, int y, unsigned int color) {

    int i;

    for(i = 0; str[i]; i++) 
        drawCharacter(b, str[i], x+(i*8), y, color);
}

void cmd_getCursor(unsigned char *x, unsigned char *y) {

    *x = cmd_x;
    *y = cmd_y;
}

void cmd_putCursor(unsigned char x, unsigned char y) {

    cmd_x = x;
    cmd_y = y;
}

void cmd_pchar(unsigned char c) {

    if(c == '\n') {

        cmd_x = 0;
        cmd_y++;
    } else {

        drawCharacter(cmd_bmp, c, (cmd_x*8), (cmd_y*12), RGB(0, 0, 0));
        cmd_x++;

        if(cmd_x > cmd_max_chars) {

            cmd_x = 0;
            cmd_y++;
        }
    }
    
    //Should update this so it only repaints the section
    //of bitmap where the character was drawn    
    if(cmd_y > cmd_max_lines)
        cmd_clear();        
}

void cmd_prints(unsigned char* s) {

    while(*s)
        cmd_pchar(*s++);
}

void cmd_clear() {

    unsigned int x, y;

    for(y = 0; y < cmd_height; y++)
        for(x = 0; x < cmd_width; x++)
            cmd_bmp->data[y*cmd_bmp->width + x] = RGB(255, 255, 255);
            
    cmd_x = 0;
    cmd_y = 0;
    
    repaintAll(cmd_window, cmd_bmp);
    
    //Now clear to green temporarily to see what's getting repainted and where
    for(y = 0; y < cmd_height; y++)
        for(x = 0; x < cmd_width; x++)
            cmd_bmp->data[y*cmd_bmp->width + x] = RGB(0, 255, 0);
}

void cmd_printDecimal(unsigned int dword) {

    unsigned char digit[12];
    int i, j;

    i = 0;
    while(1) {

        if(!dword) {

            if(i == 0)
                digit[i++] = 0;

            break;
        }

        digit[i++] = dword % 10;
        dword /= 10;
    }

    for(j = i - 1; j >= 0; j--)
        cmd_pchar(digit[j] + '0');
}

void cmd_printHexByte(unsigned char byte) {

    cmd_pchar(digitToHex((byte & 0xF0)>>4));
    cmd_pchar(digitToHex(byte & 0xF));
}


void cmd_printHexWord(unsigned short wd) {

    cmd_printHexByte((unsigned char)((wd & 0xFF00)>>8));
    cmd_printHexByte((unsigned char)(wd & 0xFF));
}


void cmd_printHexDword(unsigned int dword) {

    cmd_printHexWord((unsigned short)((dword & 0xFFFF0000)>>16));
    cmd_printHexWord((unsigned short)(dword & 0xFFFF));
}

void cmd_scans(int c, char* b) {

    unsigned char temp_char;
    int index = 0;

    for(index = 0 ; index < c-1 ; ) {
        temp_char = getch();

        if(temp_char != 0) {
            b[index] = temp_char;
            cmd_pchar(b[index]);

            if(b[index] == '\n') {
                b[index] = 0;
                break;
            }

            index++;

            if(index == c-1)
                cmd_pchar('\n');
        }
    }

    b[index+1] = 0;
}


void cmd_init(unsigned int win) {

    window* tmpwnd;
    
    cmd_window = win;
    cmd_bmp = getWindowContext(cmd_window);
    cmd_x = 0;
    cmd_y = 0;
    tmpwnd = getWindowByHandle(cmd_window);
    cmd_bx = tmpwnd->x;
    cmd_by = tmpwnd->y;
    cmd_width = cmd_bmp->width;
    cmd_height = cmd_bmp->height;
    cmd_max_chars = (cmd_width/8) - 1;
    cmd_max_lines = (cmd_height/12) - 1;
    cmd_clear();
}

void testMain() {
	
	makeWindows();
}
