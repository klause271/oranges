
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "log.h"
#include "syslog.h"
static char buf[64];
PRIVATE void block(struct proc* p);
PRIVATE void unblock(struct proc* p);
PRIVATE int  msg_send(struct proc* current, int dest, MESSAGE* m);
PRIVATE int  msg_receive(struct proc* current, int src, MESSAGE* m);
PRIVATE int  deadlock(int src, int dest);
PRIVATE const char* get_syscall_name(int type);

/*****************************************************************************
 *                                schedule
 *****************************************************************************/
/**
 * <Ring 0> Choose one proc to run.
 * 
 *****************************************************************************/
PUBLIC void schedule()
{
	struct proc*	p;
	int		greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = &FIRST_PROC; p <= &LAST_PROC; p++) {
			if (p->p_flags == 0) {
				if (p->ticks > greatest_ticks) {
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}
		}

		if (!greatest_ticks)
			for (p = &FIRST_PROC; p <= &LAST_PROC; p++)
				if (p->p_flags == 0)
					p->ticks = p->priority;
	}
}

/*****************************************************************************
 *                                sys_sendrec
 *****************************************************************************/
/**
 * <Ring 0> The core routine of system call `sendrec()'.
 * 
 * @param function SEND or RECEIVE
 * @param src_dest To/From whom the message is transferred.
 * @param m        Ptr to the MESSAGE body.
 * @param p        The caller proc.
 * 
 * @return Zero if success.
 *****************************************************************************/
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p)
{
	assert(k_reenter == 0);	/* make sure we are not in ring0 */
	assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) ||
	       src_dest == ANY ||
	       src_dest == INTERRUPT);

	int ret = 0;
	int caller = proc2pid(p);
	MESSAGE* mla = (MESSAGE*)va2la(caller, m);
	mla->source = caller;

	assert(mla->source != src_dest);
	// 在系统调用执行前记录日志
	if (system_ready && 
		src_dest != TASK_LOG && 
		m->type != LOG_MESSAGE) {
		// 写入缓冲区
		struct syscall_log* log = &syscall_logs[syscall_log_index];
		strcpy(log->proc_name, p->name);
		log->pid = caller;
		strcpy(log->syscall_name, get_syscall_name(m->type));
		log->ret = 0;
		log->valid = 1;
		
		syscall_log_index = (syscall_log_index + 1) % MAX_SYSCALL_LOGS;
	}

	/**
	 * Actually we have the third message type: BOTH. However, it is not
	 * allowed to be passed to the kernel directly. Kernel doesn't know
	 * it at all. It is transformed into a SEND followed by a RECEIVE
	 * by `send_recv()'.
	 */
	if (function == SEND) {
		ret = msg_send(p, src_dest, m);
		if (ret != 0)
			return ret;
	}
	else if (function == RECEIVE) {
		ret = msg_receive(p, src_dest, m);
		if (ret != 0)
			return ret;
	}
	else {
		panic("{sys_sendrec} invalid function: "
		      "%d (SEND:%d, RECEIVE:%d).", function, SEND, RECEIVE);
	}

	return 0;
	// 如果系统调用失败，记录错误
	if (system_ready && 
		src_dest != TASK_LOG && 
		m->type != LOG_MESSAGE && 
		ret != 0) {
		// 写入缓冲区
		struct syscall_log* log = &syscall_logs[syscall_log_index];
		strcpy(log->proc_name, p->name);
		log->pid = caller;
		strcpy(log->syscall_name, get_syscall_name(m->type));
		log->ret = ret;
		log->valid = 1;
		
		syscall_log_index = (syscall_log_index + 1) % MAX_SYSCALL_LOGS;
	}

	return ret;
}

/*****************************************************************************
 *				  ldt_seg_linear
 *****************************************************************************/
/**
 * <Ring 0~1> Calculate the linear address of a certain segment of a given
 * proc.
 * 
 * @param p   Whose (the proc ptr).
 * @param idx Which (one proc has more than one segments).
 * 
 * @return  The required linear address.
 *****************************************************************************/
PUBLIC int ldt_seg_linear(struct proc* p, int idx)
{
	struct descriptor * d = &p->ldts[idx];

	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

/*****************************************************************************
 *				  va2la
 *****************************************************************************/
/**
 * <Ring 0~1> Virtual addr --> Linear addr.
 * 
 * @param pid  PID of the proc whose address is to be calculated.
 * @param va   Virtual address.
 * 
 * @return The linear address for the given virtual address.
 *****************************************************************************/
PUBLIC void* va2la(int pid, void* va)
{
	struct proc* p = &proc_table[pid];

	u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	u32 la = seg_base + (u32)va;

	if (pid < NR_TASKS + NR_NATIVE_PROCS) {
		assert(la == (u32)va);
	}

	return (void*)la;
}

/*****************************************************************************
 *                                reset_msg
 *****************************************************************************/
/**
 * <Ring 0~3> Clear up a MESSAGE by setting each byte to 0.
 * 
 * @param p  The message to be cleared.
 *****************************************************************************/
PUBLIC void reset_msg(MESSAGE* p)
{
	memset(p, 0, sizeof(MESSAGE));
}

/*****************************************************************************
 *                                block
 *****************************************************************************/
/**
 * <Ring 0> This routine is called after `p_flags' has been set (!= 0), it
 * calls `schedule()' to choose another proc as the `proc_ready'.
 *
 * @attention This routine does not change `p_flags'. Make sure the `p_flags'
 * of the proc to be blocked has been set properly.
 * 
 * @param p The proc to be blocked.
 *****************************************************************************/
PRIVATE void block(struct proc* p)
{
	assert(p->p_flags);
	schedule();
}

/*****************************************************************************
 *                                unblock
 *****************************************************************************/
/**
 * <Ring 0> This is a dummy routine. It does nothing actually. When it is
 * called, the `p_flags' should have been cleared (== 0).
 * 
 * @param p The unblocked proc.
 *****************************************************************************/
PRIVATE void unblock(struct proc* p)
{
	assert(p->p_flags == 0);
}

/*****************************************************************************
 *                                deadlock
 *****************************************************************************/
/**
 * <Ring 0> Check whether it is safe to send a message from src to dest.
 * The routine will detect if the messaging graph contains a cycle. For
 * instance, if we have procs trying to send messages like this:
 * A -> B -> C -> A, then a deadlock occurs, because all of them will
 * wait forever. If no cycles detected, it is considered as safe.
 * 
 * @param src   Who wants to send message.
 * @param dest  To whom the message is sent.
 * 
 * @return Zero if success.
 *****************************************************************************/
PRIVATE int deadlock(int src, int dest)
{
	struct proc* p = proc_table + dest;
	while (1) {
		if (p->p_flags & SENDING) {
			if (p->p_sendto == src) {
				/* print the chain */
				p = proc_table + dest;
				printl("=_=%s", p->name);
				do {
					assert(p->p_msg);
					p = proc_table + p->p_sendto;
					printl("->%s", p->name);
				} while (p != proc_table + src);
				printl("=_=");

				return 1;
			}
			p = proc_table + p->p_sendto;
		}
		else {
			break;
		}
	}
	return 0;
}

/*****************************************************************************
 *                                msg_send
 *****************************************************************************/
/**
 * <Ring 0> Send a message to the dest proc. If dest is blocked waiting for
 * the message, copy the message to it and unblock dest. Otherwise the caller
 * will be blocked and appended to the dest's sending queue.
 * 
 * @param current  The caller, the sender.
 * @param dest     To whom the message is sent.
 * @param m        The message.
 * 
 * @return Zero if success.
 *****************************************************************************/
PRIVATE int msg_send(struct proc* current, int dest, MESSAGE* m)
{
	struct proc* sender = current;
	struct proc* p_dest = proc_table + dest; /* proc dest */

	assert(proc2pid(sender) != dest);

	/* check for deadlock here */
	if (deadlock(proc2pid(sender), dest)) {
		panic(">>DEADLOCK<< %s->%s", sender->name, p_dest->name);
	}

	if ((p_dest->p_flags & RECEIVING) && /* dest is waiting for the msg */
	    (p_dest->p_recvfrom == proc2pid(sender) ||
	     p_dest->p_recvfrom == ANY)) {
		assert(p_dest->p_msg);
		assert(m);

		phys_copy(va2la(dest, p_dest->p_msg),
			  va2la(proc2pid(sender), m),
			  sizeof(MESSAGE));
		p_dest->p_msg = 0;
		p_dest->p_flags &= ~RECEIVING; /* dest has received the msg */
		p_dest->p_recvfrom = NO_TASK;
		unblock(p_dest);

		assert(p_dest->p_flags == 0);
		assert(p_dest->p_msg == 0);
		assert(p_dest->p_recvfrom == NO_TASK);
		assert(p_dest->p_sendto == NO_TASK);
		assert(sender->p_flags == 0);
		assert(sender->p_msg == 0);
		assert(sender->p_recvfrom == NO_TASK);
		assert(sender->p_sendto == NO_TASK);
	}
	else { /* dest is not waiting for the msg */
		sender->p_flags |= SENDING;
		assert(sender->p_flags == SENDING);
		sender->p_sendto = dest;
		sender->p_msg = m;

		/* append to the sending queue */
		struct proc * p;
		if (p_dest->q_sending) {
			p = p_dest->q_sending;
			while (p->next_sending)
				p = p->next_sending;
			p->next_sending = sender;
		}
		else {
			p_dest->q_sending = sender;
		}
		sender->next_sending = 0;

		block(sender);

		assert(sender->p_flags == SENDING);
		assert(sender->p_msg != 0);
		assert(sender->p_recvfrom == NO_TASK);
		assert(sender->p_sendto == dest);
	}

	return 0;
}


/*****************************************************************************
 *                                msg_receive
 *****************************************************************************/
/**
 * <Ring 0> Try to get a message from the src proc. If src is blocked sending
 * the message, copy the message from it and unblock src. Otherwise the caller
 * will be blocked.
 * 
 * @param current The caller, the proc who wanna receive.
 * @param src     From whom the message will be received.
 * @param m       The message ptr to accept the message.
 * 
 * @return  Zero if success.
 *****************************************************************************/
PRIVATE int msg_receive(struct proc* current, int src, MESSAGE* m)
{
	struct proc* p_who_wanna_recv = current; /**
						  * This name is a little bit
						  * wierd, but it makes me
						  * think clearly, so I keep
						  * it.
						  */
	struct proc* p_from = 0; /* from which the message will be fetched */
	struct proc* prev = 0;
	int copyok = 0;
	disable_int();
	assert(proc2pid(p_who_wanna_recv) != src);

	if ((p_who_wanna_recv->has_int_msg) &&
	    ((src == ANY) || (src == INTERRUPT))) {
		/* There is an interrupt needs p_who_wanna_recv's handling and
		 * p_who_wanna_recv is ready to handle it.
		 */

		MESSAGE msg;
		reset_msg(&msg);
		msg.source = INTERRUPT;
		msg.type = HARD_INT;

		assert(m);

		phys_copy(va2la(proc2pid(p_who_wanna_recv), m), &msg,
			  sizeof(MESSAGE));

		p_who_wanna_recv->has_int_msg = 0;

		assert(p_who_wanna_recv->p_flags == 0);
		assert(p_who_wanna_recv->p_msg == 0);
		assert(p_who_wanna_recv->p_sendto == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);
		
		enable_int();
		return 0;
	}


	/* Arrives here if no interrupt for p_who_wanna_recv. */
	if (src == ANY) {
		/* p_who_wanna_recv is ready to receive messages from
		 * ANY proc, we'll check the sending queue and pick the
		 * first proc in it.
		 */
		if (p_who_wanna_recv->q_sending) {
			p_from = p_who_wanna_recv->q_sending;
			copyok = 1;

			assert(p_who_wanna_recv->p_flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			assert(p_who_wanna_recv->p_sendto == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);
			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
		}
	}
	else if (src >= 0 && src < NR_TASKS + NR_PROCS) {
		/* p_who_wanna_recv wants to receive a message from
		 * a certain proc: src.
		 */
		p_from = &proc_table[src];

		if ((p_from->p_flags & SENDING) &&
		    (p_from->p_sendto == proc2pid(p_who_wanna_recv))) {
			/* Perfect, src is sending a message to
			 * p_who_wanna_recv.
			 */
			copyok = 1;

			struct proc* p = p_who_wanna_recv->q_sending;

			assert(p); /* p_from must have been appended to the
				    * queue, so the queue must not be NULL
				    */

			while (p) {
				assert(p_from->p_flags & SENDING);

				if (proc2pid(p) == src) /* if p is the one */
					break;

				prev = p;
				p = p->next_sending;
			}

			assert(p_who_wanna_recv->p_flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			assert(p_who_wanna_recv->p_sendto == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);
			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
		}
	}

	if (copyok) {
		/* It's determined from which proc the message will
		 * be copied. Note that this proc must have been
		 * waiting for this moment in the queue, so we should
		 * remove it from the queue.
		 */
		if (p_from == p_who_wanna_recv->q_sending) { /* the 1st one */
			assert(prev == 0);
			p_who_wanna_recv->q_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}
		else {
			assert(prev);
			prev->next_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}

		assert(m);
		assert(p_from->p_msg);

		/* copy the message */
		phys_copy(va2la(proc2pid(p_who_wanna_recv), m),
			  va2la(proc2pid(p_from), p_from->p_msg),
			  sizeof(MESSAGE));

		p_from->p_msg = 0;
		p_from->p_sendto = NO_TASK;
		p_from->p_flags &= ~SENDING;
		unblock(p_from);
	}
	else {  /* nobody's sending any msg */
		/* Set p_flags so that p_who_wanna_recv will not
		 * be scheduled until it is unblocked.
		 */
		p_who_wanna_recv->p_flags |= RECEIVING;

		p_who_wanna_recv->p_msg = m;
		p_who_wanna_recv->p_recvfrom = src;
		block(p_who_wanna_recv);

		assert(p_who_wanna_recv->p_flags == RECEIVING);
		assert(p_who_wanna_recv->p_msg != 0);
		assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
		assert(p_who_wanna_recv->p_sendto == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);
	}

	enable_int();
	return 0;
}

/*****************************************************************************
 *                                inform_int
 *****************************************************************************/
/**
 * <Ring 0> Inform a proc that an interrupt has occured.
 * 
 * @param task_nr  The task which will be informed.
 *****************************************************************************/
PUBLIC void inform_int(int task_nr)
{
	struct proc* p = proc_table + task_nr;

	if ((p->p_flags & RECEIVING) && /* dest is waiting for the msg */
	    ((p->p_recvfrom == INTERRUPT) || (p->p_recvfrom == ANY))) {
		p->p_msg->source = INTERRUPT;
		p->p_msg->type = HARD_INT;
		p->p_msg = 0;
		p->has_int_msg = 0;
		p->p_flags &= ~RECEIVING; /* dest has received the msg */
		p->p_recvfrom = NO_TASK;
		assert(p->p_flags == 0);
		unblock(p);

		assert(p->p_flags == 0);
		assert(p->p_msg == 0);
		assert(p->p_recvfrom == NO_TASK);
		assert(p->p_sendto == NO_TASK);
	}
	else {
		p->has_int_msg = 1;
	}
}

/*****************************************************************************
 *                                dump_proc
 *****************************************************************************/
PUBLIC void dump_proc(struct proc* p)
{
	char info[STR_DEFAULT_LEN];
	int i;
	int text_color = MAKE_COLOR(GREEN, RED);

	int dump_len = sizeof(struct proc);

	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, 0);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, 0);

	sprintf(info, "byte dump of proc_table[%d]:\n", p - proc_table); disp_color_str(info, text_color);
	for (i = 0; i < dump_len; i++) {
		sprintf(info, "%x.", ((unsigned char *)p)[i]);
		disp_color_str(info, text_color);
	}

	/* printl("^^"); */

	disp_color_str("\n\n", text_color);
	sprintf(info, "ANY: 0x%x.\n", ANY); disp_color_str(info, text_color);
	sprintf(info, "NO_TASK: 0x%x.\n", NO_TASK); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);

	sprintf(info, "ldt_sel: 0x%x.  ", p->ldt_sel); disp_color_str(info, text_color);
	sprintf(info, "ticks: 0x%x.  ", p->ticks); disp_color_str(info, text_color);
	sprintf(info, "priority: 0x%x.  ", p->priority); disp_color_str(info, text_color);
	/* sprintf(info, "pid: 0x%x.  ", p->pid); disp_color_str(info, text_color); */
	sprintf(info, "name: %s.  ", p->name); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "p_flags: 0x%x.  ", p->p_flags); disp_color_str(info, text_color);
	sprintf(info, "p_recvfrom: 0x%x.  ", p->p_recvfrom); disp_color_str(info, text_color);
	sprintf(info, "p_sendto: 0x%x.  ", p->p_sendto); disp_color_str(info, text_color);
	/* sprintf(info, "nr_tty: 0x%x.  ", p->nr_tty); disp_color_str(info, text_color); */
	disp_color_str("\n", text_color);
	sprintf(info, "has_int_msg: 0x%x.  ", p->has_int_msg); disp_color_str(info, text_color);
}


/*****************************************************************************
 *                                dump_msg
 *****************************************************************************/
PUBLIC void dump_msg(const char * title, MESSAGE* m)
{
	int packed = 0;
	printl("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",  //, (0x%x, 0x%x, 0x%x)}",
	       title,
	       (int)m,
	       packed ? "" : "\n        ",
	       proc_table[m->source].name,
	       m->source,
	       packed ? " " : "\n        ",
	       m->type,
	       packed ? " " : "\n        ",
	       m->u.m3.m3i1,
	       m->u.m3.m3i2,
	       m->u.m3.m3i3,
	       m->u.m3.m3i4,
	       (int)m->u.m3.m3p1,
	       (int)m->u.m3.m3p2,
	       packed ? "" : "\n",
	       packed ? "" : "\n"/* , */
		);
}

// 检查栈的返回地址
PUBLIC int sys_checkstack()
{
	int i;
	struct proc* p = proc_table;
	// 遍历所有进程
	for(i = 0; i < NR_TASKS + NR_PROCS; i++, p++)
	{
		if(p->p_flags == FREE_SLOT)	//如果进程未使用
			continue;
		u32 ebp = p->regs.ebp;
		u32 retaddress_offset = ebp + 4;
		u32 ss = p->regs.ss;
		u32 base = reassembly(p->ldts[ss >> 3].base_high, 24,
						  	  p->ldts[ss >> 3].base_mid, 16,
						  	  p->ldts[ss >> 3].base_low);
		u32 retaddr = *(u32*)(retaddress_offset + base);
		if(i < NR_TASKS + NR_NATIVE_PROCS)
		{
			if(retaddr > (u32)task_stack)
			{
				printf("name:%s, detect stack overflow!\n", p->name);
				printf("reaturn address:%x\n",retaddr);
				assert(0);
			}
		}
		else
		{
			//printf("name:%s,%x  %x\n", p->name, base,retaddr);
			if(retaddr > 0x20420)
			{
				printf("name:%s, detect stack overflow!\n", p->name);
				assert(0);
			}
		}
	}
	return 0;
}

PRIVATE const char* get_syscall_name(int type)
{
	switch(type) {
		case 0:            return "MSG_INIT";
		
		// 硬件中断
		case HARD_INT:      return "HARD_INT";
		
		// SYS任务相关
		case GET_TICKS:     return "GET_TICKS";
		case GET_PID:       return "GET_PID";
		case GET_RTC_TIME:  return "GET_RTC_TIME";
		
		// 文件系统相关
		case OPEN:          return "OPEN";
		case CLOSE:         return "CLOSE";
		case READ:          return "READ";
		case WRITE:         return "WRITE";
		case LSEEK:         return "LSEEK";
		case STAT:          return "STAT";
		case UNLINK:        return "UNLINK";
		
		// 进程控制相关
		case SUSPEND_PROC:  return "SUSPEND_PROC";
		case RESUME_PROC:   return "RESUME_PROC";
		case EXEC:          return "EXEC";
		case WAIT:          return "WAIT";
		case FORK:          return "FORK";
		case EXIT:          return "EXIT";
		
		// 系统调用返回
		case SYSCALL_RET:   return "SYSCALL_RET";
		
		// 设备驱动相关
		case DEV_OPEN:      return "DEV_OPEN";
		case DEV_CLOSE:     return "DEV_CLOSE";
		case DEV_READ:      return "DEV_READ";
		case DEV_WRITE:     return "DEV_WRITE";
		case DEV_IOCTL:     return "DEV_IOCTL";
		
		default:            
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "UNKNOWN(%d)", type);
			return buf;
	}
}

PUBLIC int sys_manage_log(int operation, int param)
{
    switch(operation) {
        case 1:  // enable level
            enable_log_level(param);
            break;
        case 2:  // disable level
            disable_log_level(param);
            break;
        case 3:  // enable category
            enable_log_category(1 << (param-1));
            break;
        case 4:  // disable category
            disable_log_category(1 << (param-1));
            break;
        case 5:  // disable all and clear logs
            // 禁用所有日志级别和类别
            log_level = 0;
            log_categories = 0;
            
            // 清空所有日志缓冲区
            for (int i = 0; i < MAX_SYSCALL_LOGS; i++) {
                syscall_logs[i].valid = 0;
            }
            for (int i = 0; i < MAX_DEVICE_LOGS; i++) {
                device_logs[i].valid = 0;
            }
            for (int i = 0; i < MAX_SWITCH_LOGS; i++) {
                switch_logs[i].from_pid = 0;
            }
            break;
        default:
            return -1;
    }
    return 0;
}



/*****************************************************************************
 *                             sys_canary_check
 *****************************************************************************/

PUBLIC void sys_canary_check() {
    struct proc *p = p_proc_ready;
	
    // 只对用户进程检查
    if (p - proc_table >= NR_TASKS + NR_NATIVE_PROCS) {
        if (strcmp(p->name, "attack_stack") == 0) return;
        if (canary_enabled == 0) return;
        
        int offset_canary = p->regs.ebp - 16;
        int ss = p->regs.ss;
        int base = reassembly(
            p->ldts[ss >> 3].base_high, 24,
            p->ldts[ss >> 3].base_mid, 16,
            p->ldts[ss >> 3].base_low
        );
        unsigned int canary_address = offset_canary + base;
        unsigned int canary = *(unsigned int *)(canary_address);


        if (canary != 0xffffffff) {
			printl("Stack overflow occurred in process\n");
            //printl("Stack overflow occurred in process\n");
			//printl("canary%x\n",canary);
        }
        return;
    }
}
