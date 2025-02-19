/*************************************************************************//**
 *****************************************************************************
 * @file   systask.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   2007
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
#include "keyboard.h"
#include "proto.h"

PRIVATE int read_register(char reg_addr);
PRIVATE u32 get_rtc_time(struct time *t);

void clear_process(struct proc* p, int pid) {
    // 清理进程的资源，例如文件描述符、内存等
   
    memset(p, 0, sizeof(struct proc));  // 将进程表条目清零
    p->p_flags = FREE_SLOT;             // 标记为未占用
    p->ldt_sel = SELECTOR_LDT_FIRST + pid * sizeof(struct descriptor); // 重置 LDT
   
}

/*****************************************************************************
 *                                task_sys
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK SYS.
 * 
 *****************************************************************************/
PUBLIC void task_sys()
{
	MESSAGE msg;
	struct time t;

	while (1) {
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;

		switch (msg.type) {
		case KILL_PROC: {
    			int pid = msg.PID;

    			// 只允许kill pid为6-8的进程
    			if (pid < 6 || pid > 8) {
        			msg.type = ERROR;  // 非法的PID
        			send_recv(SEND, src, &msg);
        			break;
    			}

    			struct proc* p = &proc_table[pid];

   			 // 检查进程是否已被标记
    			if (p->p_flags != FREE_SLOT) {
        			p->p_flags = 0x03;  // 使用特殊标记隐藏该进程 (0x02 表示隐藏)
        			msg.type = OK;      // 返回成功状态
    			} else {
        			msg.type = ERROR;   // 已经是空槽
    			}

    			send_recv(SEND, src, &msg);
    			break;
		}


		case GET_TICKS:
			msg.RETVAL = ticks;
			send_recv(SEND, src, &msg);
			break;
		case GET_PID:
			msg.type = SYSCALL_RET;
			msg.PID = src;
			send_recv(SEND, src, &msg);
			break;
		case GET_RTC_TIME:
			msg.type = SYSCALL_RET;
			get_rtc_time(&t);
			phys_copy(va2la(src, msg.BUF),
				  va2la(TASK_SYS, &t),
				  sizeof(t));
			send_recv(SEND, src, &msg);
			break;
		case GET_PROC_INFO:
			msg.type = SYSCALL_RET;
			phys_copy(va2la(src, msg.BUF),
				  va2la(TASK_SYS, &proc_table[msg.PID]),
				  sizeof(struct proc));
			send_recv(SEND, src, &msg);
            break;
		default:
			panic("unknown msg type");
			break;
		}
	}
}


/*****************************************************************************
 *                                get_rtc_time
 *****************************************************************************/
/**
 * Get RTC time from the CMOS
 * 
 * @return Zero.
 *****************************************************************************/
PRIVATE u32 get_rtc_time(struct time *t)
{
	t->year = read_register(YEAR);
	t->month = read_register(MONTH);
	t->day = read_register(DAY);
	t->hour = read_register(HOUR);
	t->minute = read_register(MINUTE);
	t->second = read_register(SECOND);

	if ((read_register(CLK_STATUS) & 0x04) == 0) {
		/* Convert BCD to binary (default RTC mode) */
		t->year = BCD_TO_DEC(t->year);
		t->month = BCD_TO_DEC(t->month);
		t->day = BCD_TO_DEC(t->day);
		t->hour = BCD_TO_DEC(t->hour);
		t->minute = BCD_TO_DEC(t->minute);
		t->second = BCD_TO_DEC(t->second);
	}

	t->year += 2000;

	return 0;
}

/*****************************************************************************
 *                                read_register
 *****************************************************************************/
/**
 * Read register from CMOS.
 * 
 * @param reg_addr 
 * 
 * @return 
 *****************************************************************************/
PRIVATE int read_register(char reg_addr)
{
	out_byte(CLK_ELE, reg_addr);
	return in_byte(CLK_IO);
}




