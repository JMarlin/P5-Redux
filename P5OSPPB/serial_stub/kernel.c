#include "../core/global.h"
#include "../ascii_io/ascii_o.h"
#include "../core/util.h"
#include "../core/ascii_io/serial.h"
#include "../serialfs/serialfs.h"

int main(void) {

    __asm__ ("cli");

    //We should relocate ourselves to a different memory address and then jump there
    //because we need to load the incoming kernel at the location this kernel was 
    //loaded to

    setColor(0x1F);
    initScreen(0);
    clear();
    prints("Initializing serial port...");
    initSerial(SERIAL_DEFAULT_BASE);
    sfs_init(serReceived, serGetch, serPutch);
    prints("Done\nWaiting for server to pick up...");

    if(!sfs_connect()) {

        prints("Timeout\n");
        while(1);
    }

    prints("Done\nLooking for boot image...");

    if(!sfs_fileExists(":p5kern.bin")) {

        prints("File not found.\n");
        while(1);
    }

    int file_size = sfs_getFileSize(":p5kern.bin");
    int file_read = 0;
    int read_size = 0;
    int rx_id = sfs_init_transfer(":p5kern.bin");
    unsigned char rx_buf[512];

    prints("Found\nReceiving file.");

    while(1) {

        read_size = sfs_transfer(rx_id, rx_buf, 512);

        if(read_size < 0) {

            prints("Error receiving file.");
            while(1);
        }

        prints(".");

        //copy rx_buf into load area here

        file_read += read_size;
        
        if(file_read >= file_size)
            break;
    }

    prints("Done\nEntering loaded kernel\n");

    //Jump to kernel load point
}
