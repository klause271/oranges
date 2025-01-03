#include "type.h"  
#include "stdio.h"  
#include "const.h"  
#include "protect.h"  
#include "string.h"  
#include "fs.h"  
#include "proc.h"  
#include "tty.h"  
#include "console.h"  
#include "global.h"  
#include "keyboard.h"  
#include "proto.h"  
  
PUBLIC void kill(char* name) {  
    MESSAGE msg;
    msg.type = KILL;
    msg.STATUS = 0;
    strcpy(msg.Buf, name);
    send_recv(BOTH, TASK_MM, &msg);  
}  
