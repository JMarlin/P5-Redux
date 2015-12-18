#ifndef KEY_H
#define KEY_H 

#define KEY_MSG_CLASS ((unsigned int)0x00200000)

#define KEY_GETCH (KEY_MSG_CLASS | 1)

int initKey();
unsigned char getch(void);

#endif //KEY_H