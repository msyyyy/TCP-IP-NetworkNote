#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <string>

#define BUF_SIZE 1024
void error_handling(char *message);
char *calc(std::string s);
char res[10];

int main(int argc,char *argv[])
{
    int serv_sock,clnt_sock;
    char message[BUF_SIZE];
    int str_len,i;

    struct sockaddr_in serv_adr,clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc!=2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET,SOCK_STREAM,0);
    if(serv_sock==-1)
        error_handling((char *)"socket() error");
    
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port= htons(atoi(argv[1]));

    if(bind(serv_sock,(struct sockaddr *)&serv_adr,sizeof(serv_adr))==-1)
        error_handling((char *)"bind() error");
    
    if(listen(serv_sock,5)==-1)
        error_handling((char *)"listen() error");
    
    clnt_adr_sz = sizeof(clnt_adr);

    clnt_sock = accept(serv_sock,(struct sockaddr *)&clnt_adr, &clnt_adr_sz);
    if(clnt_sock==-1)
        error_handling((char *)"accept() error");
    
    str_len = read(clnt_sock,message,BUF_SIZE);
    write(clnt_sock,calc(message),str_len);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
char *calc(std::string s)
{
    std::istringstream strm(s);
    int n;
    int a[20];
    char p;
    strm >> n;
    for(int i=0;i<n;i++)
        strm >> a[i];
    strm>>p;
    switch (p)
    {
    case '+':
        for(int i=1;i<n;i++)
            a[0]+=a[i];
        break;
    case '-':
        for(int i=1;i<n;i++)
            a[0]-=a[i];
        break;
    case '*':
         for(int i=1;i<n;i++)
            a[0]*=a[i];
        break;
    }
    std::stringstream strm1;
    strm1<<a[0];
    std::string ss;
    strm1>>ss;
    char *s1=(char *)ss.c_str();
    return s1;

}