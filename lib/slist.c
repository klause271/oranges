/*************************************************************************//**
 *****************************************************************************
 * @file   list.c
 *****************************************************************************
 *****************************************************************************/

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
#include "proto.h"

/*****************************************************************************
 *                                slist
 *****************************************************************************/

PUBLIC char* slist(char *path)
{
	MESSAGE msg;
	msg.type = SEARCH;
    	memcpy(msg.Buf, path, strlen(path));
    	send_recv(BOTH, TASK_FS, &msg);

	return msg.Buf;
}
