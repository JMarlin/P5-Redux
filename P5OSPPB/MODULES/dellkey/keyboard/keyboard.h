//KBC registers
#define KBC_DREG 0x60 //IO Port for the data register (r/w)
#define KBC_SREG 0x64 //O Port for the status register (r)
#define KBC_CREG 0x64 //I Port for the command register (w)

//KBC Status Register bit fields 
#define SR_OBSTAT 0x1 //Output buffer status bit (0 empty/1 full)
#define SR_IBSTAT 0x2 //Input buffer status bit (0 empty/1 full)
#define SR_CMDDAT 0x8 //Bit selecting if dreg is for PS/2(0) or KBC(1) 
#define SR_TOERR 0x40 //Bit selecting if KBC should timeout(1) or not(0)
#define SR_PARERR 0x80 //Bit selecting if KBC parity checks(1) or not(0)

//KBC Command Register command messages
#define KBC_READ_CCB 0x20 //Read the controller config byte from the KBC
#define KBC_WRITE_CCB 0x60 //Write the next byte to the CCB
#define KBC_EN_PORT2 0xA8 //Enable second PS/2 port (if supported)
#define KBC_DIS_PORT2 0xA7 //Disable second PS/2 port (if supported)
#define KBC_TST_PORT2 0xA9 //Run a self-test on second PS/2 port
#define KBC_SELFTEST 0xAA //Test the keyboard controller  
#define KBC_TST_PORT1 0xAB //Run a self-test on first PS/2 port
#define KBC_DIS_PORT1 0xAD //Disable the first PS/2 port
#define KBC_EN_PORT1 0xAE //Enable the first PS/2 port
#define KBC_CPYLOW 0xC1 //Copy the low nybble of inport to SR high nybble
#define KBC_CPYHI 0xC2 //Copy the high nybble of inport to SR high nybble
#define KBC_READ_COP 0xD0 //Read the controller output port
#define KBC_WRITE_COP 0xD1 //Write next byte to controller out port
#define KBC_WRITE_PORT2 0xD4 //Write next byte to second PS/2 port
#define KBC_RESET_CPU 0xF1 //Pulses CPU reset line low for 6ms

//KBC Controller Configuration Byte bit fields
#define CCB_PORT1_INT 0x1 //Enable(1) or disable(0) PS/2 port 1 interrupt
#define CCB_PORT2_INT 0x2 //Enable(1) or disable(0) PS/2 port 2 interrupt
#define CCB_SYSFLAG 0x4 //If 1, system passed POST
#define CCB_PORT1_CLK 0x10 //Enable(1) or disable(0) PS/2 port 1 clock
#define CCB_PORT2_CLK 0x20 //Enable(1) or disable(0) PS/2 port 2 clock
#define CCB_PORT1_TRANS 0x40 //En(1) or dis(0) scancode translation

//KBC Controller Output Port bit fields
#define COP_SYSRST 0x1 //Reset the system. ALWAYS SET TO 1.
#define COP_A20 0x2 //Enable/disable the A20 gate
#define COP_PORT2_CLK 0x4 //Status of PS/2 port 2 clock line
#define COP_PORT2_DATA 0x8 //Status of PS/2 port 2 data line
#define COP_PORT1_INB 0x10 //True if there is a byte from PS/2 1 available
#define COP_PORT2_INB 0x20 //True if there is a byte from PS/2 2 available
#define COP_PORT1_CLK 0x40 //Status of PS/2 port 1 clock line
#define COP_PORT1_DATA 0x80 //Status of PS/2 port 1 data line

//KBC command return codes
#define PORT_TST_PASS 0x00 //Port 1 or 2 selftest all clear
#define PORT_TST_CLKLOW 0x01 //Clock line stuck low
#define PORT_TST_CLKHI 0x02 //Clock line stuck high
#define PORT_TST_DATLOW 0x03 //Data line stuck low
#define PORT_TST_DATHI 0x04 //Data line stuck high
#define KBC_TST_PASS 0x55 //KBC self test passed
#define KBC_TST_FAIL 0xFC //KBC self test failed

//PS/2 commands
#define PS2_RESET 0xFF //Reset the device
#define PS2_SETCODESET 0xF0 //Set the scancode set

//PS/2 command return codes
#define PS2_OK 0xFA //ACK or Success
#define PS2_FAIL 0xFC //Command failed

//Function declarations
void keyboard_sendCommand(unsigned char command);
unsigned char keyboard_getStatus();
void keyboard_inputWait();
void keyboard_outputWait();
unsigned char keyboard_getData();
int keyboard_init();
int keyboard_setScancodeSet(unsigned char setNumber);
void keyboard_debug();
void setupKeyTable();
