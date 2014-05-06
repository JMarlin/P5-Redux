bool clearA20()
{
        
}

void outb(unsigned short _port, unsigned char _data)
{
       asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}
