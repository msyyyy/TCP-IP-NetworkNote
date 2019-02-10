#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock;
    struct sockaddr_in serv_adr;
    pid_t pid;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0); //创建服务端套接字

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    pid =fork();
    if(pid==0)
    {
        printf("子进程的 serv_sock：%d\n", serv_sock);
    }
    else 
    {
        printf("父进程的 serv_sock：%d\n", serv_sock);
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}