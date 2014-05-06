#include "ascii_o.h"
#include "../CORE/commands.h"
#include "../CORE/util.h"

char* screenBase;
int cursor_x, cursor_y;
char color_code;

void initScreen(){
        color_code = 0x07;
        screenBase = (char*)0xB8000;
        setCursor(0, 0);
        return;
}

void setCursor(int x, int y)
{
        int linear_location = (y*80)+x;
        unsigned short* video_port = (unsigned short*)0x0463;
        cursor_x = x;
        cursor_y = y;
        outb(video_port[1], 0x0F);
        outb(video_port[1]+1, (unsigned char)(linear_location&0xFF));
        outb(video_port[1], 0x0E);
        outb(video_port[1]+1, (unsigned char)((linear_location>>8)&0xFF));
        return;
}

void setColor(char newCode)
{
        color_code = newCode;
        return;
}

void scrollScreen()
{
        setCursor(0, 0);
        return;
}

void pchar(char _inin)
{
        if(_inin != '\n'){
                screenBase[((cursor_y*80)+cursor_x)*2] = _inin; //Insert the character
                screenBase[(((cursor_y*80)+cursor_x)*2)+1] = color_code; //Grey on black
                cursor_x++;
        }

        if(cursor_x == 80 || _inin == '\n'){
                cursor_x = 0;
                cursor_y++;
                if(cursor_y == 25){
                        scrollScreen();
                }
        }

        setCursor(cursor_x, cursor_y);

        return;

//The below is the BIOS method, no longer usable in protected mode
/*
  asm("movb $0x0E, %ah\n\t"
      "movw $0x0000, %bx\n");
  asm("movb %0, %%al\n" : : "q"(_inin) : "memory", "%eax");
  asm("int $0x10\n");
*/
}


void prints(char* _str)
{      
  int index = 0;
  while(_str[index] != 0)
  {
        pchar(_str[index]);
        index++;
  }
}



