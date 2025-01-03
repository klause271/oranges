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

int equal(char* a, char* b)
{
    int i = 0;
    int is_equal = 1;
    if (strlen(a) != strlen(b)) {
        return 0;
    }
    for (i = 0; i < strlen(a); i++) {
        if (a[i] != b[i]) {
            is_equal = 0;
            break;
        }
    }
    return is_equal;
}

PRIVATE void cleanup(struct proc * proc)
{
	MESSAGE msg2parent;
	msg2parent.type = SYSCALL_RET;
	msg2parent.PID = proc2pid(proc);
	msg2parent.STATUS = proc->exit_status;
	send_recv(SEND, proc->p_parent, &msg2parent);

	proc->p_flags = FREE_SLOT;
}

PUBLIC void do_kill()
{
	int i, pid = -1, parent_pid = -1;
    struct proc *p = proc_table;
    for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++) {
        if (equal(p->name, mm_msg.Buf)) {
            pid = i;
            parent_pid = proc_table[pid].p_parent;
            break;
        }
    }
	if (pid == -1) {
		return;
	}
	p = &proc_table[pid];

	/* tell FS, see fs_exit() */
	MESSAGE msg2fs;
	msg2fs.type = KILL;
	msg2fs.PID = pid;
	send_recv(BOTH, TASK_FS, &msg2fs);

	free_mem(pid);

	p->exit_status = 0;

	if (proc_table[parent_pid].p_flags & WAITING) { /* parent is waiting */
		proc_table[parent_pid].p_flags &= ~WAITING;
		cleanup(&proc_table[pid]);
	}
	else { /* parent is not waiting */
		proc_table[pid].p_flags |= HANGING;
	}

	/* if the proc has any child, make INIT the new parent */
	proc_table[pid].p_parent = INIT;
	if ((proc_table[INIT].p_flags & WAITING) &&
		(proc_table[pid].p_flags & HANGING)) {
		proc_table[INIT].p_flags &= ~WAITING;
		cleanup(&proc_table[pid]);
	}
}
