#include "../include/p5.h"
#include "../include/gfx.h"
#include "../include/pci.h"
#include "../include/wyg.h"

#define CMD_COUNT 6

//Function declarations
void usrClear(void);
void consVer(void);
void usrExit(void);
void cpuUsage(void);
void startGui(unsigned short xres, unsigned short yres);
void pciList(void);
void doBmp(void);
void cmd_pchar(unsigned char c);
void cmd_prints(unsigned char* s);
void cmd_clear();
void cmd_getCursor(unsigned char *x, unsigned char *y);
void cmd_putCursor(unsigned char x, unsigned char y);
void cmd_printHexByte(unsigned char byte);
void cmd_printHexWord(unsigned short wd);
void cmd_printHexDword(unsigned int dword);
void cmd_printDecimal(unsigned int dword);

//Typedefs
typedef void (*sys_command)(void);

//Variable declarations
char* cmdWord[CMD_COUNT] = {
    "CLR",
    "VER",
    "EXIT",
    "CPU",
    "PCI",
    "BMP"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&usrClear,
    (sys_command)&consVer,
    (sys_command)&usrExit,
    (sys_command)&cpuUsage,
    (sys_command)&pciList,
    (sys_command)&doBmp
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
    
    //Make two windows
    window_a = createWindow(300, 200, WIN_FIXEDSIZE);
    window_b = createWindow(300, 200, WIN_FIXEDSIZE);
    
    //Install them into the root window
    installWindow(window_a, ROOT_WINDOW);
    installWindow(window_b, ROOT_WINDOW);
    
    //Make them prettily cascade
    moveWindow(window_a, 100, 100);
    moveWindow(window_b, 150, 150);    
    
    //Make them visible
    showWindow(window_a);
    showWindow(window_b);
    
    //Hang
    while(1);
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

void drawCharacter(char c, int x, int y, unsigned int color) {

    setCursor(x, y);
    setColor(color);
    drawChar(c);
}


void drawCharacterBold(char c, int x, int y, unsigned int color) {

    setColor(color);
    setCursor(x, y);
    drawChar(c);
    setCursor(x+1, y);
    drawChar(c);
    setCursor(x, y+1);
    drawChar(c);
    setCursor(x+1, y+1);
    drawChar(c);
}


void drawString(char* str, int x, int y, unsigned int color) {

    int i;

    setColor(color);

    for(i = 0; str[i]; i++) {

        setCursor(x+(i*8), y);
        drawChar(str[i]);
    }
}


void drawPanel(int x, int y, int width, int height, unsigned int color, int border_width, int invert) {

    unsigned char r = RVAL(color);
    unsigned char g = GVAL(color);
    unsigned char b = BVAL(color);
    unsigned int light_color = RGB(r > 195 ? 255 : r + 60, g > 195 ? 255 : g + 60, b > 195 ? 255 : b + 60);
    unsigned int shade_color = RGB(r < 60 ? 0 : r - 60, g < 60 ? 0 : g - 60, b < 60 ? 0 : b - 60);
    unsigned int temp;
    int i;

    if(invert) {

        temp = shade_color;
        shade_color = light_color;
        light_color = temp;
    }

    for(i = 0; i < border_width; i++) {

        //Top edge
        setCursor(x+i, y+i);
        setColor(light_color);
        drawHLine(width-(2*i));

        //Left edge
        setCursor(x+i, y+i+1);
        drawVLine(height-((i+1)*2));

        //Bottom edge
        setCursor(x+i, (y+height)-(i+1));
        setColor(shade_color);
        drawHLine(width-(2*i));

        //Right edge
        setCursor(x+width-i-1, y+i+1);
        drawVLine(height-((i+1)*2));
    }

    //Fill
    setCursor(x+border_width, y+border_width);
    setColor(color);
    fillRect(width-(2*border_width), height-(2*border_width));
}


void drawButton(unsigned char* text, int x, int y, int width, int height, unsigned int color, int border_width) {

    int str_len, lines, cols, lmargin, tmargin, l, c, ocols;

    for(str_len = 0; text[str_len]; str_len++);

    lines = (str_len * 8) / (width - border_width*2);

    //Do a ceiling
    if((str_len*8) % width)
        lines++;

    cols = (width - border_width*2) / 8;
    ocols = cols;
    lmargin = (width - border_width*2 - (cols*8)) / 2;
    tmargin = (height - border_width*2 - (lines*12)) / 2;

    drawPanel(x, y, width, height, color, border_width, 0);

    for(l = 0; l < lines; l++) {

        //Recalculate values for the last lines in case it's
        //shorter than the rest
        if(l == (lines - 1)) {

            cols = str_len - (cols * (lines - 1));
            lmargin = (width - border_width*2 - (cols*8)) / 2;
        }

        for(c = 0; c < cols; c++) {

            drawCharacterBold(text[c+(l*ocols)], x + lmargin + border_width + (c*8), y +  border_width + tmargin + (l*12), 0);
        }
    }
}

void printDecimal(unsigned int dword) {

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
        pchar(digit[j] + '0');
}

unsigned char cmd_x;
unsigned char cmd_y;
int cmd_width;
int cmd_height;
int cmd_max_chars;

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

        drawCharacter(c, (cmd_x*8)+27, (cmd_y*12)+27, RGB(0, 255, 0));
        cmd_x++;

        if(cmd_x > cmd_max_chars) {

            cmd_x = 0;
            cmd_y++;
        }
    }
}

void cmd_clchar(unsigned char c) {

    if(c == '\n') {

        cmd_x = 0;
        cmd_y++;
    } else {

        drawCharacter(c, (cmd_x*8)+27, (cmd_y*12)+27, RGB(0, 0, 0));
        cmd_x++;

        if(cmd_x > cmd_max_chars) {

            cmd_x = 0;
            cmd_y++;
        }
    }
}

void cmd_prints(unsigned char* s) {

    while(*s)
        cmd_pchar(*s++);
}

void cmd_printClear(int count) {

    setCursor((cmd_x*8) + 27, (cmd_y*12) + 27);
    setColor(0);
    fillRect(8*count, 12);
    cmd_x += count;
}

void cmd_clear() {

    setCursor(27, 27);
    setColor(0);
    fillRect(cmd_width, cmd_height);
    cmd_x = 0;
    cmd_y = 0;
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


void cmd_init(unsigned short xres, unsigned short yres) {

    cmd_x = 0;
    cmd_y = 0;
    cmd_width = xres-114;
    cmd_height = yres-54;
    cmd_max_chars = (cmd_width/8) - 1;
}

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

void doBmp(void) {

    int x, y, redraw;
    unsigned char tmp_chr = 0;
    bitmap* test_bmp = newBitmap(64, 64);

    if(!test_bmp) {

        cmd_prints("Couldn't allocate a new bitmap!\n");
        return;
    }

    //cmd_prints("Bitmap created\nCreating gradient");

    for(x = 0; x < 64; x++) {

        for(y = 0; y < 64; y++) {

            //cmd_pchar('.');
            test_bmp->data[y * 64 + x] = RGB(0, 0, (((y / 4) & 0xFF) << 4) | ((x / 4) & 0xFF));
        }
    }

    x = 50;
    y = 50;
    setCursor(x, y);
    drawBitmap(test_bmp);

    while((tmp_chr = getch()) != 'Q') {

        if(tmp_chr) {
         
            redraw = 1;
         
            switch(tmp_chr) {
            
                case 'W':
                    cmd_pchar('^');
                    if(y > 0) y--;
                    break;
                    
                case 'S':
                    cmd_pchar('v');
                    y++;
                    break;
                    
                case 'A':
                    cmd_pchar('<');
                    if(x > 0) x--;
                    break;
                    
                case 'D':
                    cmd_pchar('>');
                    x++;
                    break;
            
                default:
                    redraw = 0;
                    break;    
            }   
            
            if(redraw) {
                
                setCursor(x, y);
                drawBitmap(test_bmp);
            }
        }
    }
}

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

void startGui_old(unsigned short xres, unsigned short yres) {

    int i, os_build;
    unsigned char tmpch;

    //Backdrop
    setCursor(0, 0);
    setColor(RGB(11, 162, 193));
    fillRect(xres, yres);

    //System Bar
    drawPanel(xres-60, 0, 60, yres, RGB(200, 200, 200), 2, 0);

    //Indent for fun
    drawButton("P5OS", xres-55, 5, 50, 50, RGB(200, 200, 200), 2);

    //Redundant text for shadow
    drawString("Build #", 1, 1, 0);
    os_build = getBuildNumber();
    drawCharacter((os_build >> 28 & 0xF) > 9 ? (os_build >> 28 & 0xF) - 10 + 'A' : (os_build >> 28 & 0xF) + '0', 57, 1, 0);
    drawCharacter((os_build >> 24 & 0xF) > 9 ? (os_build >> 24 & 0xF) - 10 + 'A' : (os_build >> 24 & 0xF) + '0', 65, 1, 0);
    drawCharacter((os_build >> 20 & 0xF) > 9 ? (os_build >> 20 & 0xF) - 10 + 'A' : (os_build >> 20 & 0xF) + '0', 73, 1, 0);
    drawCharacter((os_build >> 16 & 0xF) > 9 ? (os_build >> 16 & 0xF) - 10 + 'A' : (os_build >> 16 & 0xF) + '0', 81, 1, 0);
    drawCharacter((os_build >> 12 & 0xF) > 9 ? (os_build >> 12 & 0xF) - 10 + 'A' : (os_build >> 12 & 0xF) + '0', 89, 1, 0);
    drawCharacter((os_build >> 8 & 0xF) > 9 ? (os_build >> 8 & 0xF) - 10 + 'A' : (os_build >> 8 & 0xF) + '0', 97, 0, 1);
    drawCharacter((os_build >> 4 & 0xF) > 9 ? (os_build >> 4 & 0xF) - 10 + 'A' : (os_build >> 4 & 0xF) + '0', 105, 1, 0);
    drawCharacter((os_build & 0xF) > 9 ? (os_build & 0xF) - 10 + 'A' : (os_build & 0xF) + '0', 113, 1, 0);

    //Text test
    drawString("Build #", 0, 0, RGB(255, 255, 255));
    os_build = getBuildNumber();
    drawCharacter((os_build >> 28 & 0xF) > 9 ? (os_build >> 28 & 0xF) - 10 + 'A' : (os_build >> 28 & 0xF) + '0', 56, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 24 & 0xF) > 9 ? (os_build >> 24 & 0xF) - 10 + 'A' : (os_build >> 24 & 0xF) + '0', 64, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 20 & 0xF) > 9 ? (os_build >> 20 & 0xF) - 10 + 'A' : (os_build >> 20 & 0xF) + '0', 72, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 16 & 0xF) > 9 ? (os_build >> 16 & 0xF) - 10 + 'A' : (os_build >> 16 & 0xF) + '0', 80, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 12 & 0xF) > 9 ? (os_build >> 12 & 0xF) - 10 + 'A' : (os_build >> 12 & 0xF) + '0', 88, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 8 & 0xF) > 9 ? (os_build >> 8 & 0xF) - 10 + 'A' : (os_build >> 8 & 0xF) + '0', 96, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 4 & 0xF) > 9 ? (os_build >> 4 & 0xF) - 10 + 'A' : (os_build >> 4 & 0xF) + '0', 104, 0, RGB(255, 255, 255));
    drawCharacter((os_build & 0xF) > 9 ? (os_build & 0xF) - 10 + 'A' : (os_build & 0xF) + '0', 112, 0, RGB(255, 255, 255));

    //body of 'window'
    drawPanel(20, 20, xres-100, yres-40, RGB(200, 200, 200), 2, 0);

    //'window' inner bevel
    drawPanel(25, 25, xres-110, yres-50, RGB(200, 200, 200), 2, 1);

    //'terminal' background
    setCursor(27, 27);
    setColor(0);
    fillRect(xres-114, yres-54);

    //Simple input loop
    cmd_init(xres, yres);

    cmd_prints("----| Welcome to P5 |----\n");

    //Wait for keypress
    //NOTE: BIG ISSUE HERE IS THAT THIS LOOP IS FILLING UP THE KERNEL HEAP
    //SPACE WITH MESSAGE REQUESTS AND SLOWING DOWN EVERYTHING ELSE ON THE
    //SYSTEM BECAUSE OF THE CONSTANT POLLING NATURE OF SCANS
    //THIS IS ALSO A PROBLEM THAT NEEDS TO BE ADDRESSED WITH THE CURRENT
    //KMALLOC IMPLEMENTATION BEING CRAZY SLOW WHICH NEEDS TO BE ADDRESSED
    //(PROBABLY BY SWITCHING TO ALLOCATING PAGES AND KEEPING A TREE OF PAGES
    //WHICH CAN THEN BE SUBDIVIDED ON FUTURE ALLOCATIONS)
    //BUT THE FIRST, FASTEST THING WE CAN DO FOR NOW IS TO USE THE SLEEP-FOR
    //-MESSAGE CAPABILITY OF MESSAGING WE HAVE NOW TO CHANGE SCANS FROM A POLLING
    //TO A WAKE-ON-EVENT TYPE OF COMMAND. THIS WAY WE'LL ONLY SPAWN A SINGLE
    //MESSAGE AND THE PROCESS WILL BE BYPASSED BY THE SCHEDULER UNTIL THE KERNEL
    //HAS A KEYPRESS READY TO SEND BACK
    while(1) {

        cmd_prints("::");
        cmd_scans(50, inbuf);
        parse(inbuf);
    }


    //drawRect(102, 102, xres - 204, yres - 204, RGB(68, 76, 82));

    //int row;
    //int inner_height = (yres - 204);
    //int change_rate = inner_height / 255;
    //int shade_val = 255;
    //for(row = 0; row < inner_height; row++) {

        //drawHLine(102, row + 102, xres - 204, RGB(0, 0, shade_val));

        //if(!(row % change_rate))
            //shade_val--;
    //}


}
