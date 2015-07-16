#include "../include/p5.h"
#include "../include/gfx.h"

#define CMD_COUNT 3

//Function declarations
void usrClear(void);
void consVer(void);
void usrExit(void);
void startGui(unsigned short xres, unsigned short yres);
void showModes(void);
void enterMode(void);
void cmd_pchar(unsigned char c);
void cmd_prints(unsigned char* s);
void cmd_clear();

//Typedefs
typedef void (*sys_command)(void);

//Variable declarations
char* cmdWord[CMD_COUNT] = {
    "CLR",
    "VER",
    "EXIT"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&usrClear,
    (sys_command)&consVer,
    (sys_command)&usrExit
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

void main(void) {

    prints("\nUser environment for P5 build #0x");
    printHexDword(getBuildNumber());
    pchar('\n');

    if(!initGfx())
        prints("\nCould not initialize GFX!\n");

    showModes();
    enterMode();
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


void showModes(void) {


    unsigned short mode_count;
    unsigned short i;
    screen_mode* mode;

    prints("Enumerating modes...");
    mode_count = enumerateModes();
    prints("done\n");

    prints("\nAvailible modes:\n");
    for(i = 0; i < mode_count; i++) {

        mode = getModeDetails(i);
        prints("    ");
        printHexByte(i);
        prints(") ");
        printHexWord(mode->width);
        pchar('x');
        printHexWord(mode->height);
        prints(", ");
        printHexByte(mode->depth);
        prints("bpp\n");
    }
}


void enterMode(void) {

    screen_mode* mode;
    unsigned short num;

    prints("mode: ");
    while(1); //Test to see whuttup
    scans(10, inbuf);
    num = inbuf[0] > '9' ? inbuf[0] - 'A' + 10 : inbuf[0] - '0';

    if(!setScreenMode(num)) {

        prints("Could not set screen mode.\n");
        return;
    }

    mode = getModeDetails(num);

    startGui(mode->width, mode->height);
}


int cmd_x;
int cmd_y;
int cmd_width;
int cmd_height;
int cmd_max_chars;

void cmd_pchar(unsigned char c) {

    if(c == '\n') {

        cmd_x = 2;
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


void cmd_prints(unsigned char* s) {

    while(s)
        cmd_pchar(*s++);
}

void cmd_clear() {

    setCursor(27, 27);
    setColor(0);
    fillRect(cmd_width, cmd_height);
    cmd_x = 0;
    cmd_y = 0;
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



void startGui(unsigned short xres, unsigned short yres) {

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

    //Wait for keypress
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
