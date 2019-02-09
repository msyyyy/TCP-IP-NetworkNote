#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char *argv[])
{
    int status;
    pid_t pid = fork();
    if (pid == 0)
    {
        sleep(15); //用 sleep 推迟子进程的执行
        return 24;
    }
    else
    {
//-1 代表等待的是终结的任意ID的子进程，status获取返回值，WNOHANG表示没有终结的子进程返回0并退出函数
        while (!waitpid(-1, &status, WNOHANG))
        {
            sleep(3);
            puts("sleep 3 sec.");
        }
        if (WIFEXITED(status))
            printf("Child send %d \n", WEXITSTATUS(status));
    }
    return 0;
}