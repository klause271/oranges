/*************************************************************************//**
 *****************************************************************************
 * @file   syslog.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   Thu Nov 20 17:02:42 2008
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

#define LOG_BUFFER_SIZE 1024

void log(int level, const char *file, int line, const char *fmt, ...) {
    if ((level & log_level) == 0) return; // 检查日志开关

    char buf[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsprintf(buf, fmt, args);
    va_end(args);

    // 将日志信息写入磁盘或发送到其他日志系统
    syslog(buf); // 假设syslog函数已实现
}

// 定义日志宏
#define LOG_DEBUG(fmt, ...)   log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)   log(LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) log(LOG_LEVEL_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   log(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/*****************************************************************************
 *                                syslog
 *****************************************************************************/
/**
 * Write log directly to the disk by sending message to FS.
 * 
 * @param fmt The format string.
 * 
 * @return How many chars have been printed.
 *****************************************************************************/
PUBLIC int syslog(const char *fmt, ...)
{
	int i;
	char buf[STR_DEFAULT_LEN];

	va_list arg = (va_list)((char*)(&fmt) + 4); /**
						     * 4: size of `fmt' in
						     *    the stack
						     */
	i = vsprintf(buf, fmt, arg);
	assert(strlen(buf) == i);

	return disklog(buf);
}

