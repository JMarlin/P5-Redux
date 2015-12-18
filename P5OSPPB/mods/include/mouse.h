#ifndef MOUSE_H
#define MOUSE_H 

#define MOUSE_MSG_CLASS ((unsigned int)0x00700000)

#define MOUSE_REG_LISTENER (MOUSE_MSG_CLASS | 1)
#define MOUSE_SEND_UPDATE  (MOUSE_MSG_CLASS | 2)

int initMouse(); //Should register the initing process as a receiver of mouse updates\

#endif //MOUSE_H