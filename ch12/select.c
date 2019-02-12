#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#define BUF_SIZE 30

int main(int argc, char *argv[])
{
    fd_set reads, temps;
    int result, str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;

    FD_ZERO(&reads);   //初始化变量
    FD_SET(0, &reads); //将文件描述符0对应的位设置为1

    /*
    timeout.tv_sec=5;
    timeout.tv_usec=5000;
    */

    while (1)
    {
        temps = reads; // 记录原先的reads值，因为在select函数后，除了发生变换的文件描述符对应位之外，其他位置为0
        timeout.tv_sec = 5;//每次调用初始化新值
        timeout.tv_usec = 0;
        result = select(1, &temps, 0, 0, &timeout); //如果控制台输入数据，则返回大于0的数，没有而引发超时后，返回0
        if (result == -1)
        {
            puts("select error!");
            break;
        }
        else if (result == 0)
        {
            puts("Time-out!");
        }
        else
        {
            if (FD_ISSET(0, &temps)) //验证发生变化的值是否是标准输入端
            {
                str_len = read(0, buf, BUF_SIZE);
                buf[str_len] = 0;
                printf("message from console: %s", buf);
            }
        }
    }
    return 0;
}