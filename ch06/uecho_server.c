#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc,char *argv[])
{
    int serv_sock;
    char message[1024];
    int str_len;
    socklen_t clnt_adr_sz;
    struct sockaddr_in  serv_adr,clnt_adr;
    if(argc!=2)
    {
        printf("Usage: %s<port>\n",argv[0]);
        exit(1);
    }
    // 创建服务器套接字,UDP使用的数据类型为SOCK_DGRAM
    serv_sock = socket(PF_INET,SOCK_DGRAM,0);

    if(serv_sock==-1)
        error_handling("UDP socket creation eerror");

    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    
    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    while(1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        // 分配地址接收数据，不限制数据传输对象
        str_len = recvfrom(serv_sock,message,BUF_SIZE,0,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);

        sendto(serv_sock,message,str_len,0,(struct sockaddr*)&clnt_adr,clnt_adr_sz);
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