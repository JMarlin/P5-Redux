#include "../ASCII_IO/ascii_o.h"
#include "util.h"

//Warning: This trashes memory at 0x1 and 0x100001.
//As such, it should only be used during init code.
int testA20()
{

      
      unsigned char* A20TestLow = (unsigned char*)0x1600;  //Choosing this randomly just because it should be in low free RAM just past the IDT loaded by PBOOT
      unsigned char* A20TestHi = (unsigned char*)0x101600;

      //prints("Setting low byte to 0\n");
      A20TestHi[0] = 0x00;
      //prints("Setting hi byte to FF\n");
      A20TestLow[0] = 0xFF;
      //prints("Testing the difference\n");
      if(A20TestHi[0] != 0xFF)
      {
        //Setting the lower value didn't affect the higher
        //value. A20 is already enabled, so quit true.
        return 1;
      }

      return 0;

}

int enableA20()
{

        unsigned char KBCOutPortReg;
        int timeout;

        prints("[enableA20()]: Attempting to enable the A20 line.\n");        

        //See if it's already enabled
        if(testA20()) 
        {
                prints("[enableA20()]: A20 is already enabled.\n");
                return 1;
        }        
        //DON'T TRY THE BIOS METHOD, WE'RE IN 32-BIT!!!
        /*
           prints("Trying BIOS method\n");
           //Try the BIOS method
           asm volatile ("movw $0x2401, %ax\n\t"
                         "int $0x15");
        */        

        if(testA20())
        {
                prints("[enableA20()]: A20 enabled via BIOS call.\n");
                return 1;                   
        }

        prints("Trying KBC method\n");
        //Try the keyboard controller
        while(inb(0x64)&0x2);   //Wait for the KBC status reg bit 1  to clear (ready for input)
        outb(0x64, 0xAD);       //Disable keyboard
        while(inb(0x64)&0x2);
        outb(0x64, 0xD0);       //Request read from Controller Output Port register
        while(!(inb(0x64)&0x1)); //Wait for the KBC satus reg bit 0 to be set (data available)
        KBCOutPortReg = inb(0x60); //Read the output port register value
        while(inb(0x64)&0x2);
        outb(0x64, 0xD1);       //Request write to KBC output port register
        while(inb(0x64)&0x2);
        outb(0x60, KBCOutPortReg|0x2); //Write the value back with the A20 bit set
        while(inb(0x64)&0x2);
        outb(0x64, 0xAE);       //Re-enable the keyboard
        for(timeout = 0; timeout < 2000; timeout++)
        {
                if(testA20())
                {
                        prints("[enableA20()]: A20 enabled via KBC.\n");
                        return 1;
                }
        }

        prints("Trying fast A20 method\n");
        //Try fast A20 method
        asm volatile ( "inb $0x92, %al\n\t"
                       "or $0x2, %al\n\t"
                       "out %al, $0x92" );
        if(testA20())
        {
                prints("[enableA20()]: A20 enabled via Fast A20 port.");
                return 1;
        }

        prints("[enableA20()]: A20 could not be enabled.\n");
        return 0;
        
}

inline void outb(unsigned short _port, unsigned char _data)
{
       asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}

inline unsigned char inb(unsigned short _port)
{
        unsigned char data;
        asm volatile ( "inb %1, %0" : "=a"(data) : "Nd"(_port) );
        return data;       
}
