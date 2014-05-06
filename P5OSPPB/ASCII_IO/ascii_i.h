void scans(unsigned int length, char* outstr);
unsigned char getch(void);
int strcmpci(char* in1, char* in2);
int slen(char* ins);
void setupKeyTable();
typedef unsigned char (*ext_getchPtr)(void);
ext_getchPtr ext_getch;
