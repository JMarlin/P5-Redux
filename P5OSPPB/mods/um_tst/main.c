#include "../include/p5.h"
#include "../include/gfx.h"
#include "../include/pci.h"
#include "../include/wyg.h"
#include "../vesa/font.h"

#define CMD_COUNT 4

//Function declarations
void usrClear(void);
void consVer(void);
void usrExit(void);
//void cpuUsage(void);
void startGui(unsigned short xres, unsigned short yres);
void pciList(void);
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
typedef void (*sys_command)(void);

//Variable declarations
char* cmdWord[CMD_COUNT] = {
    "CLR",
    "VER",
    "EXIT",
    //"CPU",
    "PCI"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&usrClear,
    (sys_command)&consVer,
    (sys_command)&usrExit,
    //(sys_command)&cpuUsage,
    (sys_command)&pciList
};

char inbuf[50];

int strcmp(char* s1, char* s2) {

    int i;

    for(i = 0; s1[i] && s2[i]; i++)
        if(s1[i] != s2[i])
            return 0;

    if(s1[i] != s2[i])
        return 0;

    return 1;
}

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

        cmd_prints("Unknown command ");
        cmd_prints(cmdbuf);
        cmd_prints("\n");
    }
}

void makeWindows() {
    
    unsigned int window_a, window_b;
    bitmap* ctx_a;
    unsigned int x, y;
    unsigned short w, h;
    unsigned char toggle = 0;
    unsigned int tile_width = 4;
    unsigned int tile_height = 4;
    unsigned int tile_data[] = {
        0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,
        0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000
    };
    
    
    //Make two windows
    getWindowDimensions(ROOT_WINDOW, &w, &h); 
    window_a = createWindow(w - 108, h - 132, WIN_FIXEDSIZE);
    
    //Set up their titles
    setTitle(window_a, "PTerm");
    
    //Install them into the root window
    installWindow(window_a, ROOT_WINDOW);
    
    //Paint a pretty picture into window A
    ctx_a = getWindowContext(window_a);
    
    //This SHOULD tile the tile image across the window
    for(x = 0; x < w - 108; x++)
        for(y = 0; y < h - 168; y++)
            ctx_a->data[y*(w - 108) + x] = tile_data[(y%tile_height)*tile_width + (x%tile_width)];
    
    //Make them prettily cascade
    moveWindow(window_a, 54, 66);
    
    //Make them visible
    showWindow(window_a);
    
    //Set up the console commands
    cmd_init(window_a);
    
    while(1) {

        cmd_prints("::");
        cmd_scans(50, inbuf);
        parse(inbuf);
    }
}

void main(void) {

    if(!initWYG()) {
        
        prints("usr.mod could not init WYG.");
        while(1); //Hang 
    }
    
    makeWindows();
}

void usrClear(void) {

    cmd_clear();
}


void consVer(void) {

    cmd_prints("P5 usermode console build 1\n");
    cmd_prints("P5 build [need fmt print and P5 build number hook]\n");
}


void usrExit(void) {

    terminate();
}

//Wrapper for setting the blit mask for the window bitmap to a specific region before requesting redraw
void repaintAll(unsigned int handle, bitmap* h_bmp) {
    
    //Set the blitting rect 
    h_bmp->top = 0;
    h_bmp->left = 0;
    h_bmp->bottom = h_bmp->height;
    h_bmp->right = h_bmp->width;   
    
    //Redraw 
    repaintWindow(handle); 
}

//Wrapper for setting the blit mask for the window bitmap to a specific region before requesting redraw
void repaintRegion(unsigned int handle, bitmap* h_bmp, unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
    
    //Set the blitting rect 
    h_bmp->top = y;
    h_bmp->left = x;
    h_bmp->bottom = y + h;
    h_bmp->right = x + w;   
    
    //Redraw 
    repaintWindow(handle); 
}

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

bitmap* cmd_bmp;
unsigned int cmd_window;
unsigned char cmd_x;
unsigned char cmd_y;
unsigned short cmd_bx, cmd_by; 
int cmd_width;
int cmd_height;
int cmd_max_chars;
int cmd_max_lines;

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

    cmd_window = win;
    cmd_bmp = getWindowContext(cmd_window);
    cmd_x = 0;
    cmd_y = 0;
    getWindowLocation(cmd_window, &cmd_bx, &cmd_by);
    cmd_width = cmd_bmp->width;
    cmd_height = cmd_bmp->height;
    cmd_max_chars = (cmd_width/8) - 1;
    cmd_max_lines = (cmd_height/12) - 1;
    cmd_clear();
}

/*
typedef struct proc_details {
    unsigned char x;
    unsigned char y;
    unsigned int pid;
    unsigned int average;
    unsigned int last_ten[50];
    unsigned int avg_count;
} proc_details;
void cpuUsage(void) {

    unsigned char x, y;
    int i, j, proc_count;
    unsigned int current_pid;
    proc_details pd[10];
    int exit = 0;

    cmd_prints("Clearing and initializing");
    for(i = 0; i < 10; i++) {

        for(j = 0; j < 50; j++)
            pd[i].last_ten[j] = 0;

        pd[i].avg_count = pd[i].average = 0;
        cmd_pchar('.');
    }

    cmd_prints("Done.\n");
    resetPidSearch();
    i = 0;

    //Get all active PIDs
    cmd_prints("Enumerating PIDs...");
    while(i < 10) {

        if(!(current_pid = getNextPid()))
            break;

        pd[i++].pid = current_pid;
    }

    cmd_prints("(Found ");
    cmd_printDecimal(i);
    cmd_prints(")\n");
    proc_count = i;

    if(proc_count == 0)
        return;

    for(i = 0; i < proc_count; i++) {

        cmd_printHexDword(pd[i].pid);
        cmd_prints(": ");
        cmd_getCursor(&(pd[i].x), &(pd[i].y));
        cmd_prints("                                        ");
        cmd_printDecimal(getImageSize(pd[i].pid));
        cmd_prints("   \n");
    }

    while(!exit) {

        setColor(RGB(0, 255, 0));

        for(i = 0; i < proc_count; i++) {

            cmd_putCursor(pd[i].x, pd[i].y);

            //Process rolling average
            if(pd[i].avg_count == 50) {

                //Rotate the buffer to the left
                for(j = 0; j < 49; j++)
                    pd[i].last_ten[j] = pd[i].last_ten[j+1];

                pd[i].last_ten[49] = getProcessCPUUsage(pd[i].pid);
            } else {

                pd[i].last_ten[pd[i].avg_count] = getProcessCPUUsage(pd[i].pid);
                pd[i].avg_count++;
            }

            pd[i].average = 0;

            for(j = 0; j < pd[i].avg_count; j ++)
                pd[i].average += pd[i].last_ten[j];

            pd[i].average /= pd[i].avg_count;

            setCursor((pd[i].x * 8) + 27, (pd[i].y * 12) + 28);
            fillRect((200 * pd[i].average) / 100, 10);
            //cmd_printDecimal(pd[i].average);
            //cmd_prints("% ");
        }

        //Busyloop
        for(i = 0; i < 0x100; i++)
            if(getch()) exit = 1;

        if(exit) {

            cmd_pchar('\n');
            break;
        }

        setColor(RGB(0, 0, 0));

        for(i = 0; i < proc_count; i++) {

            setCursor((pd[i].x * 8) + 27, (pd[i].y * 12) + 28);
            fillRect(200, 10);
            cmd_printClear(10);
        }
    }
}
*/

void PCIPrintConfig(pci_address device) {

    cmd_pchar('[');
    pci_config config = pciGetDeviceConfig(device);
    cmd_pchar(']');

	//Print the PCI address
    cmd_pchar('(');
    cmd_printHexByte((unsigned char)pciGetBus(device));
    cmd_pchar(',');
    cmd_printHexByte((unsigned char)pciGetDevice(device));
    cmd_pchar(',');
    cmd_printHexByte((unsigned char)pciGetFunction(device));
    cmd_pchar(')');

	//Print the device summary
    cmd_prints(" VID: ");
    cmd_printHexWord(config.vendor_id);
    cmd_prints(", DID: ");
    cmd_printHexWord(config.device_id);
    cmd_prints(", CC: ");
    cmd_printHexByte(config.class_code);
    cmd_prints(", SC: ");
    cmd_printHexByte(config.subclass);
    cmd_prints(", PIF: ");
    cmd_printHexByte(config.prog_if);
    cmd_prints(", REV: ");
    cmd_printHexByte(config.revision_id);
    cmd_pchar('\n');
}

void pciList(void) {

    unsigned int i, devcount;

    if(!pciInit()) {

        cmd_prints("Could not open PCI subsystem.\n");
        return;
    }

    cmd_prints("\nDetected the following ");
    devcount = pciDeviceCount();
    cmd_printDecimal(devcount);
    cmd_prints(" PCI devices: \n");

    for(i = 0; i < devcount; i++)
        PCIPrintConfig((pci_address)i);

    cmd_pchar('\n');
}
