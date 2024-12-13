#include "type.h"
#include "stdio.h"
#include "string.h"
#include "sys/const.h"
#include "sys/protect.h"
#include "sys/fs.h"
#include "sys/proc.h"
#include "sys/tty.h"
#include "sys/console.h"
#include "sys/global.h"
#include "sys/proto.h"

int my_atoi(const char* str) {
    int result = 0;
    int sign = 1;

    // 跳过空白字符
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    // 处理正负号
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // 转换数字部分
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}


int main(int args, char* argv[]) {
    if (args != 2) {
        // 检查参数数量
        printf("Usage: kill <PID>\n");
        return 1;
    }

    int pid = my_atoi(argv[1]); // 将参数转换为整数
    if (pid < NR_TASKS || pid >= NR_TASKS + NR_PROCS) {
        // 检查PID是否合法
        printf("Invalid PID: %d\n", pid);
        return 1;
    }

    // 准备消息
    MESSAGE msg;
    msg.PID = pid;
    msg.type = KILL_PROC;

    // 发送消息给系统任务模块
    send_recv(BOTH, TASK_SYS, &msg);

    // 检查返回值
    if (msg.type == OK) {
        printf("Process %d terminated successfully.\n", pid);
    } else {
        printf("Failed to terminate process %d. Error code: %d\n", pid, msg.type);
    }

    return 0;
}

