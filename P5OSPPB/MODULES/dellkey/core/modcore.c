//#include "..\..\..\SYSLIB\INCLUDE\P5SYS.H"
#include "..\KEYBOARD\KEYBOARD.H"

//This file is the main entry point for the kernel in initializing and
//installing the driver module. For now, we're kind of cheating; We'll
//create a proper driver model later on, but for now there will be a hard
//hook defined in the kernel which will intercept a keypress.
//However, it's going to be pretty cool nonetheless that we could build
//multiple keyboard driver modules which can handle specific hardware
//such as dellkey versus bochskey (which is half the point of why dynamically
//loadable modules were even a priority, so that the peculiarities of the
//development machine can be dealt with without having to make integral
//changes to the kernel code which would end up being painfully tied to
//system architecture and hardly agile at all.

//One of the important symbols in a P5 module is the mname string,
//which defines to the kernel, and through that the end user, what on earth
//kind of thing we're even loading here in the first place.
unsigned char mname[] = "Dell Precision M60 keyboard driver";
unsigned char keyTable[132];

//The next most critical symbol -- and the only other part of the object
//besides the actual object format itself and the mname string -- is the
//modInit function. This is the routine which is called by the kernel for
//all modules (though not executables, it should be noted) which allows them
//to do first time startup tasks such as, in the case of this driver,
//populating critical data structures and resetting the associated device.
int modInit(void) {

        //This sets up the array which maps scancodes to characters.
        //Pretty much the main change made from the original in-kernel
        //keyboard driver as the dell keyboard -- probably because it's
        //actually a USB device and is only acting as a PS/2 device via
        //USB legacy mode -- only sends scancode set 1 even when the
        //KBC is requested to, and reports that it has been successfully
        //changed to, set 2.
        setupKeyTable();

        //Init the keyboard, clearly
        //It should be noted, as this is probably the first instance of
        //prints you'll probably see in perusing this code, that, for the
        //moment, the p5sys.h defined function is just a stub. In the future,
        //functions in p5sys will refer to library functions which actually
        //marshall the provided arguments into an interrupt-driven system
        //call. Until then, modules just won't really do anything output
        //wise save for a pass/fail return value.
        if(!keyboard_init()) {
                //prints("[P5]: No input device availible.\n");
                return 0;
        }

        return 1;

}

unsigned char getch(void)
{
        unsigned char tempData;
        tempData = keyboard_getData();
        while(tempData == 0xF0){
                keyboard_getData();
                tempData = keyboard_getData();
        }
        if(tempData < 132){
                return keyTable[tempData];
        }else{
                return 0;
        }

        return 0; //This should make realines cycle forever waiting for input
}
