
- [开始网络编程](#1)
    - [理解网络编程和套接字](#1-1)
    - [套接字类型与协议设置](#1-2)
    - [地址族与数据序列](#1-3)
    - [基于TCP的服务器端/客户端(1)](#1-4)
    - [基于TCP的服务器端/客户端(2)](#1-5)
    - [基于UDP的服务器端/客户端](#1-6)
    - [优雅的断开套接字连接](#1-7)
    - [域名及网络地址(DNS](#1-8)
    - [套接字的多种可选项](#1-9)
    - [多进程服务器端](#1-10)
    - [进程间通信](#1-11)
    - [I/O复用](#1-12)
    - [多种I/O函数](#1-13)
    - [多播和广播](#1-14)


<h1 id='1'>1 开始网络编程</h1>

<h2 id='1-1'>1.1 理解网络编程和套接字</h2>

```
linux 头文件 #include <sys/socket.y>
windows 头文件 #include <winsock2.h>
```

### `基于linux平台的实现`

网络编程结束连接请求的套接字创建过程为
```
1. 调用socket函数创建套接字

int socket(int domain,int type ,int protocol);

2. 调用bind函数分配IP地址和端口号

int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen);

3. 调用listen函数转化为可接收请求状态

int listen(int sockfd, int backlog);

4. 调用accept函数受理连接请求

int accept(int sockfd, struct sockaddr *addr , socklen_t *addrlen);

```
linux不区分文件和套接字
```
打开文件 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
open(const char *path , int flag);// path为文件地址， flag为文件开始模式，可能有多个，由|连接
例如 fd = open("data.txt",O_CREAT|O_WRONLY|O_TRUNC)

O_CREAT     必要时创建文件
O_TRUNC     删除全部现有数据
O_APPEND    维持现有数据，保存到后面
O_RDONLY    只读打开
O_WRONLY    只写打开
O_RDWR      读写打开

关闭文件
#include <unistd.h>
int close(int fd);// fd为文件描述符

将数据写入文件
#include <unistd.h>
ssize_t write(int fd,const void * buf ,size_t nbytes)

size_t为无符号整形(unsigned int)的别名， ssize_t是signed int 类型

读取文件中数据
#include <unistd.h>

ssize_t read(int fd,void *buf,size_t nbytes);
// fd 文件描述符 ，buf 保存接收数据缓冲地址值 nbytes 接收数据最大字节数
```

### `基于Windows平台的实现`

进行 Winsock编程时，首先调用WSAStartup函数
```
#include <winsock2.h>

int WSAStartup(WORD wVersionRequested , LPWSAData lpWSAData);
程序员要用的winsock版本信息 和 WSADATA结构体变量的地址值
```
Winsock编程的基础公式,初始化Winsock库
```
int main(int argc,char* argv[])
{
    WSADATA wsaData;
    ....
    if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)// MAKEWORD(1,2) 主版本号为1，副版本号为2，返回0x0201
        ErrorHandling("WSAStartup() error!");
    ....
    return 0;
}
```
`注销库，int WSACleanup(void);  成功时返回0，失败时返回SOCKET_ERROR`

### `基于Windows的套接字相关函数及展示`

```
SOCKET socket(int af,int type,int protocol)

int bind(SOCKET s, const struct sockaddr *name , int namelen);

int listen(SOCKET s, int backlog)

SOCKET accept(SOCKET s, struct sockaddr *addr , int * addrlen) 成功时返回套接字句柄

int connect (SOCKET s, const struct sockaddr *name ,int namelen)

关闭套接字函数，在linux中关闭文件和关闭套接字都会调用close函数，而windows中有专门关闭套接字的函数

int closesocket(SOCKET s)

```
winsock数据传输函数
```
int send(SOCKET s, const char *buf, int len ,int flags); 成功返回传输字节数
s 套接字句柄值  buf 保存待传输数据的缓冲地址值， len 传输字节数，flags 多项选项信息

和linux的 send函数相比，只多了flags参数

和send对应的 recv函数 ，接收数据
int recv(SOCKET s, const char *buf ,int len , int flags); 成功返回接收的字节数
```

<h2 id='1-2'>1.2 套接字类型与协议设置</h2>

```
int socket(int domain, int type ,int protocol)

domain : 套接字中使用的协议族信息

type: 套接字数据传输类型信息

protocol: 计算机间通信使用的协议信息
```
协议族 : 协议分类信息
```
PF_INET         IPv4互联网协议族
PF_INET6        IPv6
PF_LOCOL        本地通信的UNIX协议族
PF_PACKET       底层套接字的协议族
PF_IPX          IPX Novell协议族
```
套接字类型(type)：套接字的数据传输方式

1. 面向连接的套接字(SOCK_STREAM)

特征：可靠，按序基于字节的面向连接(一对一)的数据传输方式的套接字 

2. 面向消息的的套接字(SOCK_DGRAM)

特征: 不可靠，不按序，以数据的高速传输为目的的套接字

具体指定协议信息(protocol)

为啥需要第三个参数: 同一协议族中存在多个数据传输方式相同的协议

TCP套接字(IPPROTO_TCP) ， write函数调用次数可以和不同于read函数调用次数

<h2 id='1-3'>1.3 地址族与数据序列</h2>

### **分配给套接字的IP地址与端口号**

IP是为收发网络数据而分配给计算机的值，端口号是为区分程序中创建的套接字而分配给套接字的序号

IPv4： 4字节地址族  IPv6 ： 16字节地址族

IPv4标准的4字节IP地址分为网络地址和主机地址，且根据网络ID和主机ID所占字节的不同，分为A(0-127)，B(128-191)，C(192-223)，D，E

主机传输数据是先根据网络ID发送到相应路由器或交换机然后在根据主机ID向目标主机传递数据

端口号是在同一操作系统内区分不同套接字而设置的。不能将同一端口号分给不同套接字，但是TCp和UDP不会共用端口号，所以允许UDP和TCP使用同一端口号

### **地址信息的表示**

表示 IPV4 地址的结构体
```
struct sockaddr_in
{
    sa_family_t sin_family;  //地址族（Address Family）
    uint16_t sin_port;       //16 位 TCP/UDP 端口号,以网络字节序保存
    struct in_addr sin_addr; //32位 IP 地址
    char sin_zero[8];        //不使用,必须填充为0，使sockaddr_in和sockadd结构体保持一致
};
```

该结构体中提到的另一个结构体 in_addr 定义如下，它用来存放 32 位IP地址
```
struct in_addr
{
    in_addr_t s_addr; //32位IPV4地址
}
```

数据类型名称|数据类型说明|声明的头文件
-|-|-
int 8_t	|signed 8-bit int	|sys/types.h
uint8_t|	unsigned 8-bit int (unsigned char)	|sys/types.h
int16_t	|signed 16-bit int	|sys/types.h
uint16_t	|unsigned 16-bit int (unsigned short)|	sys/types.h
int32_t	|signed 32-bit int	|sys/types.h
uint32_t	|unsigned 32-bit int (unsigned long)	|sys/types.h
sa_family_t	|地址族（address family）	|sys/socket.h
socklen_t	|长度（length of struct）	|sys/socket.h
in_addr_t	|IP地址，声明为 uint_32_t	|netinet/in.h
in_port_t	|端口号，声明为 uint_16_t	|netinet/in.h

```
struct sockaddr
{
    sa_family_t sin_family; //地址族
    char sa_data[14];       //地址信息，包括IP地址和端口号，剩余部分填充为0
}
```

### **网络字节序和地址变换**

CPU保存数据方式有两种：1. 大端序(高位字节存放到低位地址) 2. 小端序(高位字节存放到高位地址) 

例如0x123456，大端序为 0x12345678  小端序为 0x78563412

为保证数据正常接收，电脑都是先把数组转换为`大端序`再进行网络传输。网络字节序是大端序

```
unsigned short htons(unsigned short);
unsigned short ntohs(unsigned short);
unsigned long htonl(unsigned long);
unsigned long ntohl(unsigned long);

htons 的 h 代表主机（host）字节序。
htons 的 n 代表网络（network）字节序。
s 代表 short
l 代表 long
```

```c++
#include <stdio.h>
#include <arpa/inet.h>
int main(int argc, char *argv[])
{
    unsigned short host_port = 0x1234;
    unsigned short net_port;
    unsigned long host_addr = 0x12345678;
    unsigned long net_addr;

    net_port = htons(host_port); //转换为网络字节序
    net_addr = htonl(host_addr);

    printf("Host ordered port: %#x \n", host_port);
    printf("Network ordered port: %#x \n", net_port);
    printf("Host ordered address: %#lx \n", host_addr);
    printf("Network ordered address: %#lx \n", net_addr);

    return 0;
}

假设在小端序cpu上运行
Host ordered port: 0x1234
Network ordered port: 0x3412
Host ordered address: 0x12345678
Network ordered address: 0x78563412
```

### **网络地址的初始化与分配**

sockaddr_in保存地址信息的是32位整数，我们要将点分十进制表示的IP地址转换为32位整数可以通过
```c++
#include <arpa/inet.h>
in_addr_t inet_addr(const char *string);
//成功时返回32位大端序整数，失败时返回InADDR_NONE
```
实例：
```c++
#include <stdio.h>
#include <arpa/inet.h>
int main(int argc, char *argv[])
{
    char *addr1 = "1.2.3.4";
    char *addr2 = "1.2.3.256";// 错误IP地址

    unsigned long conv_addr = inet_addr(addr1);
    if (conv_addr == INADDR_NONE)
        printf("Error occured! \n");
    else
        printf("Network ordered integer addr: %#lx \n", conv_addr);

    conv_addr = inet_addr(addr2);
    if (conv_addr == INADDR_NONE)// 错误IP地址返回INADDR_NONE
        printf("Error occured! \n");
    else
        printf("Network ordered integer addr: %#lx \n", conv_addr);
    return 0;
}

Network ordered integer addr: 0x4030201
Error occured!
```
inet_aton 函数与 inet_addr 函数在功能上完全相同，也是将字符串形式的IP地址转换成整数型的IP地址。只不过该函数用了 in_addr 结构体，且使用频率更高。
```c++
#include <arpa/inet.h>
int inet_aton(const char *string, struct in_addr *addr);
/*
成功时返回 1 ，失败时返回 0
string: 含有需要转换的IP地址信息的字符串地址值
addr: 将保存转换结果的 in_addr 结构体变量的地址值
*/
```
实例：
```c++
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
void error_handling(char *message);

int main(int argc, char *argv[])
{
    char *addr = "127.232.124.79";
    struct sockaddr_in addr_inet;

    if (!inet_aton(addr, &addr_inet.sin_addr))
        error_handling("Conversion error");
    else
        printf("Network ordered integer addr: %#x \n", addr_inet.sin_addr.s_addr);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

Network ordered integer addr: 0x4f7ce87f
```

将网络字节整数IP地址转换成点分十进制的字符串形式 inet_ntoa
```c++
#include <arpa/inet.h>
char *inet_ntoa(struct in_addr adr);
// 失败时返回-1
// 返回值是char指针要保存的话需要立刻复制字符串，下次调用后之前保存的字符串地址值失效
```
示例：
```c++
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in addr1, addr2;
    char *str_ptr;
    char str_arr[20];

    addr1.sin_addr.s_addr = htonl(0x1020304);// 转换为网络字节序
    addr2.sin_addr.s_addr = htonl(0x1010101);
    //把addr1中的结构体信息转换为字符串的IP地址形式
    str_ptr = inet_ntoa(addr1.sin_addr);// str_ptr绑定到inet_ntoa管理的内存
    strcpy(str_arr, str_ptr);
    printf("Dotted-Decimal notation1: %s \n", str_ptr);

    inet_ntoa(addr2.sin_addr);
    printf("Dotted-Decimal notation2: %s \n", str_ptr);
    printf("Dotted-Decimal notation3: %s \n", str_arr);
    return 0;
}

Dotted-Decimal notation1: 1.2.3.4
Dotted-Decimal notation2: 1.1.1.1
Dotted-Decimal notation3: 1.2.3.4
```

初始化网络地址sockaddr_in (主要针对服务器初始化)
```c++
struct sockaddr_in addr;
char *serv_ip = "211.217,168.13";          //声明IP地址族,硬编码
char *serv_port = "9190";                  //声明端口号字符串,硬编码
memset(&addr, 0, sizeof(addr));            //结构体变量 addr 的所有成员初始化为0
addr.sin_family = AF_INET;                 //制定地址族
addr.sin_addr.s_addr = inet_addr(serv_ip); //基于字符串的IP地址初始化
// 一般更常用的是，如果一台计算机有多个IP地址，那么只要端口号一致就能从不同IP获得数据
addr.sin_addr.s_addr = htonl(INADDR_ANY)   //通过常数INADDR_ANY分配IP地址，自动获得
addr.sin_port = htons(atoi(serv_port));    //基于字符串的IP地址端口号初始化,atoi是把字符串转换为整数
```
计算机IP数和计算机中安装的NIC数相同

向套接字分配网络地址,通过bind函数

```
#include<sys/socket.h>

int bind(int sockfd, struct sockaddr* myaddr, socklen_t addlen);// 成功返回0，失败返回-1
sockfd ：要分配地址信息的套接字文件描述符
myaddr: 存有地址信息的结构体变量地址值
addlen： 第二个结构体变量的长度
```
示例
```c++
int serv_sock;
struct sockaddr_in serv_addr;
char * serv_port ="9190";

/* 创建服务器端套接字(监听套接字)*/
serv_sock = socket(PF_INET,SOCK_STREAM,0);

/* 地址信息初始化 */
memset(&serv_addr,0,sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
//通过常数INADDR_ANY分配IP地址，自动获得
serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
serv_addr.sin_port = htons(atoi(serv_port));

/* 分配地址信息 */
bind(serv_sock , (struct sockaddr * )&serv_addr , sizeof(serv_addr) );
```

### `基于Windows的实现`

由于我用的是codeblock，要先点击在setting中的compiler，在其中的Linker settings点击 add，在windows/system32目录下 选择ws2_32.dll(ws2_32.dll是Windows Sockets应用程序接口， 用于支持Internet和网络应用程序。)

htons和htonl使用和Linux用法无差别
```C++
#include <stdio.h>
#include <winsock2.h>
void ErrorHandling(char* message)
{
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}
int main(int argc, char *argv[])
{
    WSADATA wsaData;    //定义库
    unsigned short host_port = 0x1234;
    unsigned short net_port;
    unsigned long host_addr = 0x12345678;
    unsigned long net_addr;

    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0) //库初始化
        ErrorHandling("WSAStartup() error!");

    net_port = htons(host_port); //转换为网络字节序
    net_addr = htonl(host_addr);

    printf("Host ordered port: %#x \n", host_port);
    printf("Network ordered port: %#x \n", net_port);
    printf("Host ordered address: %#lx \n", host_addr);
    printf("Network ordered address: %#lx \n", net_addr);
    WSACleanup();//关闭库
    return 0;
}

Host ordered port: 0x1234
Network ordered port: 0x3412
Host ordered address: 0x12345678
Network ordered address: 0x78563412
```
windows不存在inet_aton。存在inet_addr,inet_ntoa


<h2 id='1-4'>1.4 基于TCP的服务器端/客户端(1)</h2>

IP本身是面向消息的，不可靠的协议，每次传输数据时会帮助我们选择路径，IP协议无法应对数据错误。

TCP和UDP存在于IP之上，决定主机的数据传输方式，TCP协议确认后向不可靠的IP协议赋予可靠性

### **实现基于TCp的服务器端/客户端**

1. socket() 创建套接字 

2. bind() 分配套接字地址

3. listen() 等待连接请求状态

4. accept() 允许连接

5. read()/write() 数据交换

5. close() 断开连接

只有服务器调用listen进入等待连接请求状态客户端才能调用connect函数
```c++
#include <sys/socket.h>
// 成功时返回0，失败时返回-1
int listen(int sock, int backlog)
// sock 希望进入连接请求状态的套接字文件描述符
// backlog 连接请求等待队伍的长度，若为5，则表示最多使5个连接请求进入队列
```
服务器套接字会通过accept函数受理连接请求等待队列中待处理的客户端连接请求。函数调用成功后，accept内部将尝试用于数据I/O的套接字，并返回其文件描述符
```c++
#include <sys/socket.h>
// 成功时返回创建的套接字文件描述符，失败时返回-1
int accept(int sock ,struct sockaddr *addr ,socklen_t * addrlen);
sock : 服务器套接字的文字描述符

addr ： 保存发起连接请求的客户端地址信息的变量地址值，调用函数后向传递来的地址族变量参数填充客户端地址信息

addrlen : 第二个参数addr结构体的长度 ，但是存有长度的变量地址。函数调用完成后该变量即被填入客户端地址长度
```

回顾服务器端
```c++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    char message[] = "Hello World!";

    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    //调用 socket 函数创建套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    //调用 bind 函数分配ip地址和端口号
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    //调用 listen 函数将套接字转为可接受连接状态
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_addr_size = sizeof(clnt_addr);
    //调用 accept 函数受理连接请求。如果在没有连接请求的情况下调用该函数，则不会返回，直到有连接请求为止
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");
    //稍后要将介绍的 write 函数用于传输数据，若程序经过 accept 这一行执行到本行，则说明已经有了连接请求
    write(clnt_sock, message, sizeof(message));
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
```

客户端实现过程

1. sockrt() 创建套接字

2. connect() 请求连接

3. read()/write() 交换数据

4. close() 断开连接

```c++
#include <sys/socket.h>
// 成功时返回0，失败时返回-1
int connect(int sock ,struct sockaddr * servaddr ,socklen_t addrlen);

sock： 客户端套接字文件描述符

servaddr： 保存目标服务器端地址信息的变量地址值

addrlen： 以字节为单位传递已传递给第二个结构体参数servaddr的地址变量值
```
客户端调用connect函数后 ，服务器接收连接请求不意味着服务器调用accept函数而是意味服务器端把连接请求信息记录到等待队列。因此connect函数返回后并不立即进行数据交换

客户端给套接字分配IP和端口号： `何时`：调用connect函数时 ，`何地`：操作系统(内核中) ，`如何`Ip用主机Ip，端口随机

回顾客户端
```c++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[30];
    int str_len;

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    //创建套接字，此时套接字并不马上分为服务端和客户端。如果紧接着调用 bind,listen 函数，将成为服务器套接字
    //如果调用 connect 函数，将成为客户端套接字
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    //调用 connect 函数向服务器发送连接请求
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    str_len = read(sock, message, sizeof(message) - 1);
    if (str_len == -1)
        error_handling("read() error!");

    printf("Message from server : %s \n", message);
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
```

客户端调用connect函数前，服务器可能率先调用accept函数，但是服务器调用accept时进入阻塞状态直到客户端调用connect为止

### **实现迭代服务器端/客户端**

what: 服务器端将客户端传输的字符串数据原封不动的传回客户端，就行回声一样

how: 
```
1. 服务器端在同一时刻只与一个客户端相连，并提供回声服务
2. 服务器端依次向5个客户端提供服务并退出
3. 客户端接收用户输入的字符串并发送到服务器端
4. 服务器端将接收的字符串数据传回客户端，即回声
5. 服务器端与客户端之间的字符串回声一直执行到客户端输入Q为止
```

[迭代回声服务器端](./ch1-4/echo_server.c)

[迭代回声客户端](./ch1-4/echo_client.c)

在迭代回声客户端代码存在一些问题
```c++
write(sock, message, strlen(message));
str_len = read(sock, message, BUF_SIZE - 1);
message[str_len] = 0;
printf("Message from server: %s", message);
```
因为TCP不存在数据边界，多次调用的write函数传递的字符串可能一次性接收，也有可能字符串太长需要多次发送，但是客户端可能在尚未收到全部数据时就调用read函数

### **基于windows的回声服务器**
```
只需要记住四点： 
1. 通过WSAstrartup ，WSACleanup函数初始化并清楚套接字相关库

2.把数据类型和变量名切换为Windows风格

3.数据传输用recv，send函数而非read ,write函数

4. 关闭套接字用 closesocket函数而非close函数
```

### 习题

#### 1. 请你说明 TCP/IP 的 4 层协议栈，并说明 TCP 和 UDP 套接字经过的层级结构差异。

答：应用层， TCP/UDP ,IP层 ，链路层 。 差异为一个经过TCP，一个为UDP

#### 2. 请说出 TCP/IP 协议栈中链路层和IP层的作用，并给出二者关系

答：链路层： 物理链接 ，IP 选择正确能联通的路径，IP选择处能正确联通的链路，链路层则是物理上的连接

#### 3. 为何需要把 TCP/IP 协议栈分成 4 层（或7层）？开放式回答。

答：ARPANET 的研制经验表明，对于复杂的计算机网络协议，其结构应该是层次式的。分册的好处：①隔层之间是独立的②灵活性好③结构上可以分隔开④易于实现和维护⑤能促进标准化工作。

#### 4. 客户端调用 connect 函数向服务器端发送请求。服务器端调用哪个函数后，客户端可以调用 connect 函数？

答：listen函数

#### 5. 什么时候创建连接请求等待队列？它有何种作用？与 accept 有什么关系？

答：服务端调用 listen 函数后，accept函数正在处理客户端请求时， 更多的客户端发来了请求连接的数据，此时，就需要创建连接请求等待队列。以便于在accept函数处理完手头的请求之后，按照正确的顺序处理后面正在排队的其他请求。与accept函数的关系：accept函数受理连接请求等待队列中待处理的客户端连接请求。

#### 6.客户端中为何不需要调用 bind 函数分配地址？如果不调用 bind 函数，那何时、如何向套接字分配IP地址和端口号？

答：调用connect函数自动分配

<h2 id='1-5'>1.5 基于TCP的服务器端/客户端(2)</h2>

### 5.1 回声客户端的完美实现

echo_server.c中的代码回顾

```c++
while ((str_len = read(clnt_sock, message, BUF_SIZE)) != 0)
    write(clnt_sock, message, str_len);
```

echo_client.c中的代码回顾

```c++
write(sock, message, strlen(message));
str_len = read(sock, message, BUF_SIZE - 1);
```

回声客户端传输的是字符串而且是通过调用wirte函数一次性发送的。之后还调用一次read函数，期待接收自己传输的字符串，这是可能出错的。

这个问题其实很容易解决，因为可以提前接受数据的大小。若之前传输了20字节长的字符串，则再接收时循环调用 read 函数读取 20 个字节即可。既然有了解决办法，那么代码如下：

[回声客户端修改 echo_client2.c](./ch05/echo_client2.c)

![](https://s2.ax1x.com/2019/02/01/k3yOxS.jpg)

但是回声客户端如果无法预知接收数据长度时应如何收发数据？ 此时我们需要应用层协议来规定数据的边界，或提前告知数据的大小。
服务器/客户端实现过程中逐步定义的这些规矩集合就是应用层协议

现在写一个小程序来体验应用层协议的定义过程。要求：

1. 服务器从客户端获得多个数组和运算符信息。
2. 服务器接收到数字候对齐进行加减乘运算，然后把结果传回客户端。

例：

1. 向服务器传递3,5,9的同事请求加法运算，服务器返回3+5+9的结果
2. 请求做乘法运算，客户端会收到3*5*9的结果
3. 如果向服务器传递4,3,2的同时要求做减法，则返回4-3-2的运算结果。

[op_client.cpp](./ch05/op_client.cpp)

[op_server.cpp](./ch05/op_server.cpp)

编译
```
c版本
gcc op_client.c -o opclient
gcc op_server.c -o opserver
```


```
c++11版本
g++ -std=c++11 -o opserver op_server.cpp
g++ -std=c++11 -o opclient op_client.cpp 
```

运行
```
./opserver 9190
./opclient 127.0.0.1 9190
```

结果

![](https://s2.ax1x.com/2019/02/01/k37anI.png)

###  5.2 TCP原理

TCP套接字的数据收发无边界。服务器即使调用 1 次 write 函数传输 40 字节的数据，客户端也有可能通过 4 次 read 函数调用每次读取 10 字节。但此处也有一些一问，服务器一次性传输了 40 字节，而客户端竟然可以缓慢的分批接受。客户端接受 10 字节后，剩下的 30 字节在何处等候呢？

实际上，write 函数调用后并非立即传输数据， read 函数调用后也并非马上接收数据。如图所示，write 函数掉用瞬间，数据将移至输出缓冲；read 函数调用瞬间，从输入缓冲读取数据。

![](https://camo.githubusercontent.com/dc1ab28fc69f8ae9e85303adac21f25d4fe3fcc9/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31362f356333656134316364393363362e706e67)

I/O 缓冲特性可以整理如下：

- I/O 缓冲在每个 TCP 套接字中单独存在
- I/O 缓冲在创建套接字时自动生成
- 即使关闭套接字也会继续传递输出缓冲中遗留的数据
- 关闭套接字将丢失输入缓冲中的数据

假设发生以下情况，会发生什么事呢？

> 客户端输入缓冲为 50 字节，而服务器端传输了 100 字节。

因为 TCP 不会发生超过输入缓冲大小的数据传输。也就是说，根本不会发生这类问题，因为 TCP 会控制数据流。TCP 中有滑动窗口（Sliding Window）协议，用对话方式如下：

> - A：你好，最多可以向我传递 50 字节
> - B：好的
> - A：我腾出了 20 字节的空间，最多可以接受 70 字节
> - B：好的

数据收发也是如此，因此 TCP 中不会因为缓冲溢出而丢失数据。

write 函数在数据传输完成时(数据移到输出缓冲时)返回。不过TCP会保证对输出缓冲数据的传输

#### TCP内部工作原理1：与对方套接字的连接

TCP 套接字从创建到消失所经过的过程分为如下三步：

- 与对方套接字建立连接
- 与对方套接字进行数据交换
- 断开与对方套接字的连接

TCP 在实际通信中也会经过三次对话过程，因此，该过程又被称为 Three-way handshaking（三次握手）。接下来给出连接过程中实际交换的信息方式：

![](https://camo.githubusercontent.com/11cb8744eb600669a20ca457e23f189c0f05214b/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31362f356333656364656339666330342e706e67)

套接字是全双工方式工作的。也就是说，它可以双向传递数据。因此，收发数据前要做一些准备。首先请求连接的主机 A 要给主机 B 传递以下信息：

> [SYN] SEQ : 1000 , ACK:-

该消息中的 SEQ 为 1000 ，ACK 为空，而 SEQ 为1000 的含义如下：

> 现在传递的数据包的序号为 1000，如果接收无误，请通知我向您传递 1001 号数据包。

这是首次请求连接时使用的消息，又称为 SYN。SYN 是 Synchronization 的简写，表示收发数据前传输的同步消息。接下来主机 B 向 A 传递以下信息：

> [SYN+ACK] SEQ: 2000, ACK: 1001

此时 SEQ 为 2000，ACK 为 1001，而 SEQ 为 2000 的含义如下：

> 现传递的数据包号为 2000 ，如果接受无误，请通知我向您传递 2001 号数据包。

而 ACK 1001 的含义如下：

> 刚才传输的 SEQ 为 1000 的数据包接受无误，现在请传递 SEQ 为 1001 的数据包。

对于主机 A 首次传输的数据包的确认消息（ACK 1001）和为主机 B 传输数据做准备的同步消息（SEQ 2000）捆绑发送。因此，此种类消息又称为 SYN+ACK。

收发数据前向数据包分配序号，并向对方通报此序号，这都是为了防止数据丢失做的准备。通过项数据包分配序号并确认，可以在数据包丢失时马上查看并重传丢失的数据包。因此 TCP 可以保证可靠的数据传输。

通过这三个过程，这样主机 A 和主机 B 就确认了彼此已经准备就绪。

#### TCP工作原理2： 与对方主机的连接

通过第一步三次握手过程完成了数据交换准备，下面就开始正式收发数据，其默认方式如图所示：

![](https://camo.githubusercontent.com/2ab228809b93b96c9865156b7e09eefe73eddc20/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31362f356333656431613937636532622e706e67)

图上给出了主机 A 分成 2 个数据包向主机 B 传输 200 字节的过程。首先，主机 A 通过 1 个数据包发送 100 个字节的数据，数据包的 SEQ 为 1200 。主机 B 为了确认这一点，向主机 A 发送 ACK 1301 消息。

此时的 ACK 号为 1301 而不是 1201，原因在于 ACK 号的增量为传输的数据字节数。假设每次 ACK 号不加传输的字节数，这样虽然可以确认数据包的传输，但无法明确 100 个字节全都正确传递还是丢失了一部分，比如只传递了 80 字节。因此按照如下公式传递 ACK 信息：

> ACK 号 = SEQ 号 + 传递的字节数 + 1

与三次握手协议相同，最后 + 1 是为了告知对方下次要传递的 SEQ 号。下面分析传输过程中数据包丢失的情况：

![](https://camo.githubusercontent.com/ae88e2d81c7d4ce3e1b186374d208d0c24ca6459/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31362f356333656433373131383761362e706e67)

上图表示了通过 SEQ 1301 数据包向主机 B 传递 100 字节数据。但中间发生了错误，主机 B 未收到，经过一段时间后，主机 A 仍然未收到对于 SEQ 1301 的 ACK 的确认，因此试着重传该数据包。为了完成该数据包的重传，TCP 套接字启动计时器以等待 ACK 应答。若相应计时器发生超时（Time-out!）则重传。

### TCP内部工作原理3: 断开与套接字的连接

TCP 套接字的结束过程也非常优雅。如果对方还有数据需要传输时直接断掉该连接会出问题，所以断开连接时需要双方协商，断开连接时双方的对话如下：

> - 套接字A：我希望断开连接
> - 套接字B：哦，是吗？请稍后。
> - 套接字A：我也准备就绪，可以断开连接。
> - 套接字B：好的，谢谢合作

先由套接字 A 向套接字 B 传递断开连接的信息，套接字 B 发出确认收到的消息，然后向套接字 A 传递可以断开连接的消息，套接字 A 同样发出确认消息

![](https://camo.githubusercontent.com/3a59d552e3e97efc8ec5be07a90d5175bcafda27/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31362f356333656437353033633138632e706e67)

图中数据包内的 FIN 表示断开连接。也就是说，双方各发送 1 次 FIN 消息后断开连接。此过过程经历 4 个阶段，因此又称四次握手（Four-way handshaking）。SEQ 和 ACK 的含义与之前讲解的内容一致，省略。图中，主机 A 传递了两次 ACK 5001，也许这里会有困惑。其实，第二次 FIN 数据包中的 ACK 5001 只是因为接收了 ACK 消息后未接收到的数据重传的。

### 5.3 基于WINDOWS的实现

转换方式与之前相同，暂略

### 习题

> 答案仅代表本人个人观点，可能不是正确答案。

1. **请说明 TCP 套接字连接设置的三次握手过程。尤其是 3 次数据交换过程每次收发的数据内容。**

答：客户端先发送 关键字为SYN的 SEQ 首次请求连接，此时ACK为空(SYN) ，服务器发送SYN的SEQ和ACK 表示接收客户端的信息(SYN+ACK) ，客户端收到服务器的确认，发送SEQ和ACK表示确认接收(ACK)

2. **TCP 是可靠的数据传输协议，但在通过网络通信的过程中可能丢失数据。请通过 ACK 和 SEQ 说明 TCP 通过和何种机制保证丢失数据的可靠传输。**

答：SEQ表示发送的数据标号，ACK表示期望得到的数据标号，主机A收到主机B发送的ACK，说明ACK之前的数据都被正确接收，如果该ACK不是A要发送的SEQ说明有数据丢失，等到计时器超时就会重传，或者接收到3个相同ACK触发快速重传

3. **TCP 套接字中调用 write 和 read 函数时数据如何移动？结合 I/O 缓冲进行说明。**

答：write函数，数据被移至输出缓冲，read函数时，主机从输入缓冲读取数据

4. **对方主机的输入缓冲剩余 50 字节空间时，若本主机通过 write 函数请求传输 70 字节，请问 TCP 如何处理这种情况？**

答：TCP 中有滑动窗口控制协议，所以传输的时候会保证传输的字节数小于等于自己能接受的字节数。

<h2 id='1-6'>1.6 基于UDP的服务器端/客户端</h2>

### 6.1 理解UDP

UDP和TCP相比少了流控制机制，更加简洁，性能也更高，但是是不可靠的数据传输

UDP的作用是根据端口号将传给主机的数据包交付给最终的UDP套接字，而之前让立刻主机B的UDP数据包传递给主机A是IP的作用

![](https://camo.githubusercontent.com/ca25756e9fd311ed5575d7524d37413b256c6410/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31372f356333666432396337306266322e706e67)

传递压缩文件一般都使用TCP，而网络实时传递视频或音频一般使用UDP。

TCP比UDP慢的原因
- 收发数据前后进行的连接设置及清除过程
- 收发数据过程中为保证可靠性而添加的流控制

### 6.2 实现基于UDP的服务端/客户端

#### UDP中的服务端和客户端没有连接

UDP只有创建套接字和数据交换的过程，没有listen函数和accept函数

#### UDP的服务器端和客户端均只需一个套接字

TCP中，套接字是一对一的关系。若要向10个客户端提供服务，那么除了守门的套接字以为，还需要10个服务器套接字。但是UDP服务器端和客户端均只需一个套接字

![](https://camo.githubusercontent.com/4372567e2242e3d0dd4bc922dc6f3d794ded7dd6/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31372f356333666437303366336334302e706e67)

#### 基于UDP的数据I/O 函数

UDP不会保存连接状态，每次传输数据都要添加目标地址信息。

发送函数
```c++
#include <sys/socket.h>
// 成功时返回传输的字节数，失败时返回-1
ssize_t sendto(int sock,void *buff,size_t nbytes,int flags, struct sockaddr *to,socklen_t addrlen);
/*
sock    用于传输数据的UDP套接字文字描述符
buff    保存带传输数据的缓冲地址值
nbytes  待传输的数据长度，以字节为单位
flags   可选项参数，若没有则传输0
to      存有目标地址信息的sockaddr结构体变量的地址值
addrlen 传递给参数to的地址族结构体变量长度
*/
```
接收函数
```c++
#include <sys/socket.h>
// 成功时返回接收的字节数，失败时返回-1
ssize_t sendto(int sock,void *buff,size_t nbytes,int flags, struct sockaddr *from,socklen_t *addrlen);
/*
sock    用于接收数据的UDP套接字文字描述符
buff    保存带接收数据的缓冲地址值
nbytes  可接收的最大字节数，故无法超过参数buff所指的缓冲大小
flags   可选项参数，若没有则传入0
from    存有发送端地址信息的sockaddr结构体变量的地址值
addrlen 保存参数from的结构体变量长度的变量地址值
*/
```
#### 基于UDP的回声服务器端/客户端

[uecho_client.c](./ch06/uecho_client.c)

[uecho_server.c](./ch06/uecho_server.c)

编译运行
```
gcc uecho_client.c -o uechoclient
gcc uecho_server.c -o uechoserver
./uechoserver 9190
./uechoclient 127.0.0.1 9190
```
![](https://s2.ax1x.com/2019/02/04/kJFFpQ.png)

TCP客户端在connect函数时分配IP和端口，UDP在sendto函数时分配IP和端口

### 6.3 UDP的数据传输特性和调用connect函数

#### UDP数据传输中存在数据边界

TCP数据传输中没有数据边界代表着 "数据传输过程中调用I/O函数的次数不具有任何意义"

而UDP中输入函数调用次数应该和输出函数的调用次数完全一致。

例如

[bound_host1.c](./ch06/bound_host1.c)

[bound_host2.c](./ch06/bound_host2.c)

运行编译
```
gcc bound_host1.c -o host1
gcc bound_host2.c -o host2
./host1 9190
./host2 127.0.0.1 9190
```

![](https://s2.ax1x.com/2019/02/04/kJFW4S.png)

host1 是服务端，host2 是客户端，host2 一次性把数据发给服务端后，结束程序。但是因为服务端每隔五秒才接收一次，所以服务端每隔五秒接收一次消息。

**从运行结果也可以证明 UDP 通信过程中 I/O 的调用次数必须保持一致**

#### 已连接 UDP 与未连接 UDP 套接字

TCP套接字中需注册待传输数据的目标IP和端口号，UDP无需注册，因此通过sendto函数传输数据分为三个过程
- 1. 向UDP套接字注册目标IP和端口号
- 2. 传输数据
- 3. 删除UDP套接字中注册的目标地址信息

因此可以重复利用同一端口号向不同目标传递数据。这种未注册目标地址信息的套接字称为未连接套接字，而注册了目标地址的套接字称为 连接connect套接字。UDP默认为未连接套接字。

`为什么需要连接套接字？`  ：如果要和同一主机进行长时间通信，将UDP套接字变成已连接套接字会提高效率，在上述三个过程中，第一个和第三个阶段占通信过程的1/3

创建已连接UDP套接字只需要针对UDP调用connect函数
```c++
sock = socket(PF_INET, SOCK_DGRAM,0);
memset(&adr,0 ,sizeof(adr));
adr.sin_family = AF_INET;
adr.sin_addr.s_addr = ...
adr.sin_port = ...
connect(sock, (struct sockaddr *)&adr , sizeof(adr));
```
针对UDP套接字调用connect函数并不意味着要与对方UDP套接字连接，只是向UDP套接字注册目标IP和端口信息，因为指定了收发对象，因此不仅可以使用sendto,recvfrom,也能用write,read进行通信

将之前示例的[uecho_client.c](./ch06/uecho_client.c)程序改成基于UDP套接字的程序，并结合之前的[uecho_server.c](./ch06/uecho_server.c)运行

[uecho_con_client.c](./ch06/uecho_con_client.c)

![](https://s2.ax1x.com/2019/02/04/kJe09f.png)

### 6.4 基于windows的实现

暂略

### 6.5 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **UDP 为什么比 TCP 快？为什么 TCP 传输可靠而 TCP 传输不可靠？**

答： TCP比UDP多了流控制，因为UDP不存在流控制，可能文件会丢失，不能保证完全正确

2. **下面属于 UDP 特点的是？**

下面加粗的代表此句话正确

- **UDP 不同于 TCP ，不存在连接概念，所以不像 TCP 那样只能进行一对一的数据传输。**
- 利用 UDP 传输数据时，如果有 2 个目标，则需要 2 个套接字。
- UDP 套接字中无法使用已分配给 TCP 的同一端口号
- **UDP 套接字和 TCP 套接字可以共存。若需要，可以同时在同一主机进行 TCP 和 UDP 数据传输。**
- 针对 UDP 函数也可以调用 connect 函数，此时 UDP 套接字跟 TCP 套接字相同，也需要经过 3 次握手阶段。

3. **UDP 数据报向对方主机的 UDP 套接字传递过程中，IP 和 UDP 分别负责哪些部分？**

答: IP将数据传输到主机，UDP将数据从主机传输到UDP端口

4. **UDP 一般比 TCP 快，但根据交换数据的特点，其差异可大可小。请你说明何种情况下 UDP 的性能优于 TCP**

答：数据收发量小且需要频繁连接 ，TCP三次握手时间，四次挥手 时间消耗大

5. **客户端 TCP 套接字调用 connect 函数时自动分配IP和端口号。UDP 中不调用 bind 函数，那何时分配IP和端口号？**

答: 调用sendto函数时自动分配

6. **TCP 客户端必须调用 connect 函数，而 UDP 可以选择性调用。请问，在 UDP 中调用 connect 函数有哪些好处？**

答：与一个主机长时间连接时，不需要每次传输数据都经历 向UDP套接字注册目标IP和端口号， 删除UDP套接字中注册的目标IP和端口号 这个过程，这两个过程加起来占了通讯过程中1/3的时间

<h2 id='1-7'>1. 7 优雅的断开套接字连接</h2>

本章讨论如何优雅的断开套接字的连接，之前用的方法不够优雅是因为，我们是调用 close 函数或 closesocket 函数单方面断开连接的。

### 7.1 基于TCP的半关闭

#### 7.1.1 单方面断开连接带来的问题

close和closesocket 意味着完全断开连接，不仅不能发送数据也不能接收数据。

可能会出现主机A调用close后，主机B向A传输的数据就不能被A接收了。

为了解决这类问题，只关闭一部分数据交换中的使用的流(Half-close)应运而生，断开一部分连接是指，可以传输数据但是无法接收，或可以接受数据但无法传输。顾名思义就是只关闭流的一半。

#### 7.1.2 套接字和流

两台主机中通过套接字建立连接后进入可交换数据的状态，又称「流形成的状态」。也就是把建立套接字后可交换数据的状态看做一种流。

考虑以下情况：
> 一旦客户端连接到服务器，服务器将约定的文件传输给客户端，客户端收到后发送字符串「Thank you」给服务器端。

![](https://camo.githubusercontent.com/f649a3256efa2017799986dd6afea0120ff784eb/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31382f356334313263336261323564642e706e67)

#### 7.1.3 针对优雅断开的shutdown函数

用于半关闭的函数，shutdown
```c++
#include <sys/socket.h>
// 成功时返回0，失败时返回-1
int shutdown(int sock,int howto);
/*
sock    需要断开的套接字文字描述符
howto   传递断开方式信息
*/
```
第二个参数决定断开方式
- `SHUT_RD` :断开输入流
- `SHUT_WR` :断开输出流
- `SHUT_RDWR` :同时断开I/O流

#### 7.1.4 为何需要半关闭

传输完成后仍需发送或接收数据

服务器端应最后向客户端传递EOF表示文件传输结束，客户端通过函数返回值接收EOF避免和文件内容冲突。`在断开输出流时向对方主机传输EOF`

#### 7.1.5 基于半关闭的文件传输程序

![](https://camo.githubusercontent.com/cdce176536821331fcc4a264652f66a0ae217417/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31382f356334313332363238306162352e706e67)



[file_server.c](./ch07/file_server.c)

[file_client.c](./ch07/file_client.c)

编译运行
```
gcc file_client.c -o fclient
gcc file_server.c -o fserver
./fserver 9190
./fclient 127.0.0.1 9190
```
结果会有thank you

![](https://s2.ax1x.com/2019/02/04/kJlfeS.png)

### 7.2 基于WINDOS的实现 

暂略

### 7.3 习题

> 以下答案仅代表本人个人观点，可能不是正确答案

1. **解释 TCP 中「流」的概念。UDP 中能否形成流？请说明原因。**

答：两台主机中通过套接字建立连接后进入可交换数据的状态，又称「流形成的状态」。也就是把建立套接字后可交换数据的状态看做一种流。UDP没有连接过程，不能形成流

2. **Linux 中的 close 函数或 Windows 中的 closesocket 函数属于单方面断开连接的方法，有可能带来一些问题。什么是单方面断开连接？什么情形下会出现问题？**

答：单方面断开连接就是例如主机A，B . 主机A调用close函数，那么主机A既不能接收B的数据，也不能向B发送数据。问题：可能B还有一定要A接收的数据，这样A就接收不到了

3. **什么是半关闭？针对输出流执行半关闭的主机处于何种状态？半关闭会导致对方主机接收什么消息？**

答：半关闭：传输数据但是无法接收，或可以接受数据但无法传输。顾名思义就是只关闭流的一半。 处于不能发送数据只能接收数据的状态，对输出流半关闭的主机会向连接主机发送EOF，对方知道你数据发送完了

<h2 id='1-8'>1.8 域名及网络地址(DNS)</h2>

### 8.1 域名系统

DNS是对IP地址和域名进行互相转换的系统，其核心为DNS服务器

#### 8.1.1 什么是域名

提供网络服务的服务器端是通过IP地址区分的。但是几乎不可能以非常难记的IP地址形式交换服务器端地址信息，因此将容易记，易表述的域名分配并取代IP

#### 8.1.2 DNS服务器

域名是赋予服务器的虚拟地址，IP是实际地址。访问域名时，我们向DNS服务器请求该域名对应的IP

计算机的本地DNS服务器存储了一部分IP，计算机会先请求本地DNS如果该本地DNS服务器无法解析，会询问其他DNS服务器

DNS查询路线

![](https://camo.githubusercontent.com/08b3254ede924e0afc43ebef026592738c246bef/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31382f356334313835343835396165332e706e67)

DNS是层次化管理的一种分布式数据库系统

### 8.2 IP地址和域名之间的转化

### 8.2.1 程序中有必要使用域名嘛？

当网址是依赖于ISP服务提供者来维护IP地址时，系统相关的各种原因都会随时导致IP地址变更。所以我们不能直接在源代码中使用IP和端口号

因此最好运行程序时根据域名获取IP，在接入服务器，这样程序就不依赖于服务器IP地址了。

### 8.2.2 利用域名获取IP地址

使用以下函数可以通过传递字符串格式的域名获取IP地址
```c++
#include <netdb.h>
// 成功时返回hostent 结构体地址,失败时返回NULL指针
struct hostent * gethostbyname(const char * hostname);
```
hostent结构体定义
```c++
struct hostent
{
    char * h_name;          // 官方域名
    char ** h_aliases;      // 同一IP的可以绑定其他多个域名
    int h_addrtype;         // 地址族信息，若是IPv4，存有AF_INET
    int h_length;           // IP地址长度，若是IPv4 ,4 若是IPv6 ,16
    char ** h_addr_list;    // 以整数形式保存域名对应的IP地址。有可能有多个IP
}
```

![](https://camo.githubusercontent.com/7c5be5bfe5080103c6d394bb2f00d29982c6e552/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31382f356334313839386165343565382e706e67)

下面的代码通过一个例子来演示 gethostbyname 的应用，并说明 hostent 结构体变量特性。

[gethostbyname.c](./ch08/gethostbyname.c)

```
gcc gethostbyname.c -o hostname
./hostname www.baidu.com
```
![](https://s2.ax1x.com/2019/02/04/kJ3vG9.png)

观察
```c++
inet_ntoa(*(struct in_addr *)host->h_addr_list[i])
// inet_ntoa 将一个32位网络字节序的二进制IP地址转换成相应的点分十进制的IP地址
```
发现host->h_addr_list[i] 是指针

![](https://camo.githubusercontent.com/d78ed02d5e6d072002aaf68b01d32cb601eca4a6/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31382f356334313936353861373362382e706e67)

为什么h_addr_list指向的数组类型是char * 而不是 in_addr * ，因为hostent结构体也能保存IPv6地址信息，为了通用性

### 8.2.3 利用IP地址获取域名

gethostbyaddr函数利用IP地址获取域名

```c++
#include <netdb.h>
// 成功时返回hostent结构体变量地址值,失败时返回NULL指针
struct hostent * gethostbyaddr(const char *addr , socklen_t len, int family);
/*
addr :   含有IP地址信息的in_addr 结构体指针。为了能传递IPv4和IPv6，该变量类型为char *
len  :   向第一个参数传递的地址信息字节数，IPv4为4，IPv6为16
family : 传递地址族信息，IPv4为AF_INET,IPv6为AF_INET6
*/
```

[gethostbyaddr.c](./ch08/gethostbyaddr.c)

```
gcc gethostbyaddr.c -o hostaddr
./hostaddr 8.8.8.8
```
![](https://s2.ax1x.com/2019/02/04/kJ8feK.png)

### 8.3 基于WINDOWS的实现

暂略

### 8.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于DNS的说法正确的是？**

答：字体加粗的表示正确答案。

- **因为DNS存在，故可以使用域名代替IP**
- DNS服务器实际上是路由器，因为路由器根据域名决定数据的路径
- **所有域名信息并非集中与 1 台 DNS 服务器，但可以获取某一 DNS 服务器中未注册的所有地址**
- DNS 服务器根据操作系统进行区分，Windows 下的 DNS 服务器和 Linux 下的 DNS 服务器是不同的。

2. **阅读如下对话，并说明东秀的方案是否可行**

![](https://camo.githubusercontent.com/3c551c6a94dde7904c96e09dbd094dde691805b1/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31382f356334316132326633353339302e706e67)

答: 可以，DNS是分布式的

3. **再浏览器地址输入 www.orentec.co.kr ，并整理出主页显示过程。假设浏览器访问默认 DNS 服务器中并没有关于 www.orentec.co.kr 的地址信息.**

- 先向本地DNS服务器查询，有无存储www.orentec.co.kr的IP或存储有orentec.co.kr的DNS服务器或有存储co.kr的DNS服务器或有存储kr的DNS服务器，若无,本地DNS服务器访问到根DNS服务器，根DNS会向包含kr的TLD DNS服务器查询，TLD向包含 co.kr的权威服务器查询，直到查询包含orentec.co.kr的DNS服务器，这个服务器在逐级返回给上层DNS服务器，最终根DNS服务器返回给本地DNS服务器，本地DNS服务器再返回到主机

<h2 id='1-9'>1.9 套接字的多种可选项</h2>

套接字有多种特性，这些特性可以通过可选项更改

### 9.1 套接字可选项和I/O缓冲大小

#### 9.1.1 套接字多种可选项

我们之前写得程序都是创建好套接字之后直接使用的，此时通过默认的套接字特性进行数据通信，这里列出了一些套接字可选项。

协议层 |	选项名	|读取	|设置
-|-|-|-
SOL_SOCKET  |SO_SNDBUF	    |O	|O
SOL_SOCKET	|SO_RCVBUF      |O  |O
SOL_SOCKET  |SO_REUSEADDR   |O	|O
SOL_SOCKET  |SO_KEEPALIVE   |O  |O
SOL_SOCKET  |SO_BROADCAST   |O  |O
SOL_SOCKET  |SO_DONTROUTE   |O  |O
SOL_SOCKET  |SO_OOBINLINE   |O  |O
SOL_SOCKET  |SO_ERROR       |O  |X
SOL_SOCKET  |SO_TYPE	    |O  |X
IPPROTO_IP  |IP_TOS	        |O  |O
IPPROTO_IP  |IP_TTL	        |O  |O
IPPROTO_IP  |IP_MULTICAST_TTL|O	|O
IPPROTO_IP  |IP_MULTICAST_LOOP|O|O
IPPROTO_IP  |IP_MULTICAST_IF  |O|O
IPPROTO_TCP	|TCP_KEEPALIVE	  |O|O
IPPROTO_TCP |TCP_NODELAY	  |O|O
IPPROTO_TCP	|TCP_MAXSEG	      |O|O

由表可知，套接字可选项是分层的。IPPROTO_IP层可选项是IP协议相关事项，IPPROTO_TCP是TCP协议相关的事项，SOL_SOCKET层是套接字相关的通用可选项

#### 9.1.2 getsockopt  &  setsockopt

我们几乎可以针对上表中的所有可选项进行读取(Get)和设置(Set)

读取套接字可选项的函数 getsockopt
```c++
#include <sys/socket.h>
// 成功时返回0，失败时返回-1
int getsockopt(int sock ,int level, int optname, void *optval, socklen_t *optlen);
/*
sock    用于查看选项套接字文件的描述符
level   要查看的可选项的协议层
optname 要查看的可选项名
optval  保存查看结果的缓冲地址值
optlen  向第四个参数optlen传递的缓冲大小。调用函数后，该变量中保存通过第四个参数返回的可选项信息的字节数
*/
```
更改可选项调用的函数 setsockopt

```c++
#include <sys/socket.h>
// 成功时返回0，失败时返回-1
int setsockopt(int sock,int level,int optname,const void *optval, socklen_t oplen);
/*
sock    用于更改选项的套接字文件描述符
level   要更改的可选项协议层
optname 要更改的可选项名
optval  保存要更改的选项信息的缓冲地址值
optlen  向第四个参数optval传递的可选项信息的字节数
*/
```

下面的代码可以看出 getsockopt 的使用方法。下面示例用协议层为 SOL_SOCKET 、名为 SO_TYPE 的可选项查看套接字类型（TCP 或 UDP ）。

[sock_type.c](./ch09/sock_type.c)

编译运行
```
gcc sock_type.c -o sock_type
./sock_type
```
结果

![](https://s2.ax1x.com/2019/02/05/kJqARf.png)

SOCK_SREAM常数值为1，SOCK_DGRAM常数值为2。而且套接字类型只能创建时决定，以后不能再改

#### 9.1.3 SO_SNDBUF & SO_RCVBUF

创建套接字将同时产生I/O缓冲，SO_RCVBUF是输入缓冲大小相关可选项，SO_SNDBUF是输出缓冲大小相关可选项。这两个可选项既可以读取当前I/O缓冲的大小，也可以进行更改.

通过下面的实例读取创建套接字时默认的I/O缓冲大小

[get_buf.c](./ch09/get_buf.c)

编译运行
```
gcc get_buf.c -o getbuf
./getbuf
```

![](https://s2.ax1x.com/2019/02/05/kJxEzq.png)

下面的程序要更改I/O缓冲大小

[set_buf.c](./ch09/set_buf.c)

```
gcc get_buf.c -o setbuf
./setbuf
```

![](https://s2.ax1x.com/2019/02/05/kJxGS1.png)

为什么输出结果和我们要求修改的大小不同？

缓冲大小的设置需谨慎，因此不会完全安=按照我们的要求进行，只是通过调用setsockopt函数传递我们的需求，不过也大致反应了通过setsockopt函数设置的缓冲大小

### 9.2 SO_REUSEADDR

本节的SO_REUSEADDR及相关的Time-wait状态很重要

#### 9.2.1 发生地址分配错误(Binding Error)

发生于服务器端向客户端先发送FIN消息，那么如果用同一端口号重新运行服务器端，将输出“bind error”消息，在这种情况下，再过大约3分钟即可重新运行服务器端，因为先传输FIn消息的主机会有Time_wait过程

#### 9.2.2 Time_wait

![](https://camo.githubusercontent.com/e8f38f736e60411f504881a1677ef19e72994d0f/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31392f356334326462313832636164652e706e67)

假设A为服务器端，A向B发送FIN(可以想象成服务器控制台输入CTRL + C )。先发送FIN消息的主机要经过一段时间的Time_wait，套接字处于Time_wait状态时，相应端口是正在使用的状态

客户端和服务器端都有Time_wait状态，不过客户端因为每次运行都自主分配端口号所以没事

为什么有Time_wait状态？

如果A向B传递ACK消息后立刻消除套接字，如果这个ACK消息传递中丢失，那么B会认为A没接收自己的FIN消息，B试图重传而A已经处于终止状态，无法接收信息。因此主机B永远无法收到主机A传来的ACK消息

#### 9.2.3 地址再分配

Time_wait 可能会导致系统故障而紧急停止时没法尽快重启服务器端以提供服务

例如，下图演示了四次挥手时不得不延迟Time_wait过程的情况

![](https://camo.githubusercontent.com/c4da0ebf97337f79cd0aee910b0dc5d43ee5d8e0/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31392f356334326465633262613432622e706e67)

收到FIN消息的主机A会重启Time_wait计时器，所以如果网络状况不理想，Time_wait状态将持续

解决方案是更改SO_REUSEADDR的状态，默认值为0(false),意味着无法分配Time_wait状态下的套接字端口号，改为1(true)后即可

```c++
optlen = sizeof(option);
option = TRUE;
setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);
```

[reuseadr_eserver.c](./ch09/reuseadr_eserver.c)

### 9.3 TCP_NODELAY

### 9.3.1 Nagle 算法

目的： 为防止因数据包过多而发送网络过载

应用于TCP层

使用其与否会产生下图的差异

![](https://camo.githubusercontent.com/b5022068d7e59d2479f05919e16c84e8b70b6f87/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f31392f356334326531326162633562382e706e67)

使用Nagle算法和不使用的差别: 只有收到前一数据的ACK消息时，Nagle算法才发送下一数据

TCP套接字默认使用Nagle算法交换数据因此最大限度地进行缓冲，直到收到ACK。

如上图，使用Nagle算法，头字符'N'之前没有数据，因此立刻传输，之后等待字符'N'的ACK消息，等待过程中，剩下的“agle”填入输出缓冲，接下来，收到字符'N'的ACK消息，将输出缓冲的"agle"装入一个数据包发送

而不使用时，字符'N'到'e'依序传到输出缓冲。发送过程与ACK接收无关

上图中使用Nagle一共传递4个数据包，而不使用传输10个。因为即使只传输一字节的数据，也要加上`头信息`(几十字节),因此为提高网络传输效率，最好使用Nagle算法

当然上图也只是极端情况的展示，传输缓冲时不是逐字传输的

#### 9.3.2 禁用Nagle算法

不过Nagle不是什么时候都适用，根据传输数据的特性,网络流量未受太大影响时，不使用Nagle算法比使用传输要快很多。例如 `"传输大文件数据"。将文件数据传入到输出缓冲不会花太多时间`。因此，即使不使用Nagle,也会在装满输出缓冲时传输数据包，不仅不增加数据包量，反而在无需等待ACK前提下连续传输，大大提升传输速度

禁用很简单，只需要将TCP_NODELAY 改为1(True)即可

```c++
int opt_val = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val,sizeof(opt_val));
```
可以通过TCP_NODELAY的值查看Nagle算法的设置状态
```c++
int opt_val;
socklen_t opt_len;
opt_len = sizeof(opt_val);
getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val,&opt_len);
```
使用Nagle算法，opt_val变量保存0，禁用，保存1

### 9.4 基于WINDOWS的实现

暂略

### 9.5 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于 Time-wait 状态的说法正确的是？**

答: 加粗代表正确

- Time-wait 状态只在服务器的套接字中发生
- **断开连接的四次握手过程中，先传输 FIN 消息的套接字将进入 Time-wait 状态。**
- Time-wait 状态与断开连接的过程无关，而与请求连接过程中 SYN 消息的传输顺序有关
- Time-wait 状态通常并非必要，应尽可能通过更改套接字可选项来防止其发生


2. **TCP_NODELAY 可选项与 Nagle 算法有关，可通过它禁用 Nagle 算法。请问何时应考虑禁用 Nagle 算法？结合收发数据的特性给出说明。**

答: 根据传输数据的特性,网络流量未受太大影响时，不使用Nagle算法比使用传输要快很多。例如 "传输大文件数据"。因为将文件数据传入到输出缓冲不会花太多时间

<h2 id='1-10'>1.10 多进程服务器端</h2>

### 10.1 进程概念及应用

根据之前学到的内容，我们可以构建按序向第一个客户端到第一百个客户端提供服务的服务器端，不过这样第一百个的客户端需要很长时间的等待后才能得到服务，所以我们需要多并发。

#### 10.1.1 并发服务器端的实现方法

使服务器能同时向所有发起请求的客户端提供服务。而且网络程序中数据通信的时间比CPU运算时间占比更大，因此，向多个客户端提供服务是一种有效利用CPU的方式。

实现方法与模型
- 多进程服务器： 通过创建多个进程提供服务
- 多路复用服务器： 通过捆绑并统一管理I/O 对象提供服务
- 多路线服务器: 通过生成与客户端等量的线程提供服务

#### 10.1.2 理解进程(Process)

进程: 占用内存空间的正在运行的程序

#### 10.1.3 进程ID

所有进程都会从操作系统分配到ID，'进程ID'，其值为大于2的整数，1要分配给操作系统启动后的(用于协助操作系统)首个进程，因此用户进程无法得到ID值1。

通过ps指令可以查看当前运行的所有进程，该指令同时列出了PID(进程ID)，另外，通过指定a和u参数也能列出所有进程的详细信息

```
ps au
```
![](https://s2.ax1x.com/2019/02/06/kYHjeJ.png)

#### 10.1.4 通过调用fork函数创建进程

用于创建多进程服务器端的fork函数

```c++
#include <unistd.h>
// 成功时返回进程ID，失败时返回-1
pid_t fork(void);
```

fork函数将创建调用的进程副本，也就是说并非根据完全不同的程序来创建进程，而是复制正在运行的，调用fork函数的进程。另外，两个进程都经执行fork函数调用后的语句(准确来说是在fork函数返回后)。因为是同一个进程，复制相同的内存空间，之后的程序流要根据fork函数的返回值加以区分，即利用fork函数如下特点区分程序执行流程
- 父程序: fork函数返回子程序Id
- 子程序：fork函数返回0

![](https://camo.githubusercontent.com/4fb5bd2a643a26de90075dc6894e7102c2fb6ce2/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32302f356334336461353431326239302e706e67)

同上图可知，父进程通过调用fork函数的同时复制出子进程，并分别得到fork函数的返回值。但复制前，复制前父进程将全局变量gval增加到11,将局部变量lval的值增加到25,因此在这种状态下完成复制。复制完成后根据fork函数的返回类型区分父子进程。父进程将lval的值加1，但这不会影响子进程的lval值.同样，子进程将gval的值加1也不会影响到父进程的gval。因为fork函数调用后分层了两个完全不同的进程，只是二者共享同一代码而已。

程序

[fork.c](./ch10/fork.c)

```
gcc fork.c -o fork
./fork
```
![](https://s2.ax1x.com/2019/02/08/kNuZdg.png)

从运行结果可以知道，调用fork函数后，父子进程将拥有完全独立的内存结构。

### 10.2 进程和僵尸进程

#### 10.2.1 僵尸进程

进程完成工作后应该被销毁，但有些进程未被正确销毁，平白占用系统资源。

#### 10.2.2 产生僵尸进程的原因

例如用如下两个示例调用fork函数产生子进程的终止方式
- 传递参数并调用exit函数
- main函数中执行return 语句并返回值

向exit函数传递的参数值和main函数的return 语句返回的值都会传递给操作系统。`而操作系统不会销毁子进程，直到把这些值传递给产生该子进程的父进程`，处于这种状态下的进程就是僵尸进程。

如何向父进程传递这些值? 

只有父进程主动发起请求(函数调用)时，操作系统才会传递该值。也就是说父进程为主动要求获得子进程的结束状态值时，操作系统将一直保存并让子进程处于僵尸进程状态

创建僵尸进程的例子

[zombie.c](./ch10/zombie.c)

```
gcc zombie.c -o zombie
./zombie
```
![](https://s2.ax1x.com/2019/02/08/kNKPk4.png)
```
ps au
```
![](https://s2.ax1x.com/2019/02/08/kNKitJ.png)

可知父进程暂停30s，但是父子进程是同时销毁的。

后台处理是指将控制台窗口中的指令放在后台运行的方式(& 将触发后台命令)
```
./zombie &
```
就可以在同一控制台输入命令无需打开新的命令台

#### 10.2.3 销毁僵尸进程 1: 利用wait函数

为了销毁子进程，父进程应该主动请求获取子进程的返回值。

```c++
#include <sys/wait.h>
// 成功时返回终止的子进程ID，失败时返回-1
pid_t wait(int *statloc);
```
调用此函数时如果已有子进程终止，那么子进程终止时传递的返回值(exit函数的参数值，main函数的return返回值)将保存到该函数的参数所指的内存空间。但函数参数指向的单位中还包含其他信息，因此需要通过下列宏进行分类
- WIFEXITED 子进程正常终止时返回'真'(true)
- WEXITSTATUS 返回子进程的返回值

```c++
int status;
wait(&status);
if(WIFEXITED(status)) // 是正常终止的嘛
{
    puts("正常终止");
    printf("子进程返回值为  %d", WEXITSTATUS(status) );
}
```

[wait.c](./ch10/wait.c)

```
gcc wait.c -o wait
./wait
```

![](https://s2.ax1x.com/2019/02/09/kN2Eo8.png)

![](https://s2.ax1x.com/2019/02/09/kN2eJg.png)

通过调用ps au 命令可发现只有父进程，没有子进程，这是因为调用了wait函数，完全销毁了该程序，另外两个子进程终止时返回的3和7传递到了父进程

调用wait函数时，如果没有已终止的子进程，那么程序将阻塞直到有子进程终止，因此需要谨慎调用该函数

#### 10.2.4 销毁僵尸进程 2：  使用waitpid函数

```c++
#include <sys/wait.h>
// 成功时返回终止的子进程ID(或 0)，失败时返回-1
pid_t waitpid(pid_t pid, int * statloc,int options);

/*
pid 等待终止的目标子进程ID，若传递-1，则与wait函数相同，可以等待任意子进程终止
statloc 与wait函数的statloc参数具有相同含义
options 传递常量 WNOHANG ，即使没有终止的子进程也不会进入阻塞状态，而是返回0并退出函数
*/
```
调用waitpid函数时，程序不会阻塞

[waitpid.c](./ch10/waitpid.c)

```
gcc waitpid.c -o waitpid
./waitpid
```

![](https://s2.ax1x.com/2019/02/09/kNRkc9.png)

可以看出共执行了5此输出sleep，说明waitpid 函数并未阻塞

### 10.3 信号处理

子进程究竟何时终止? 调用waitpid函数后要无休止的等待吗？

#### 10.3.1 向操作系统求助

子进程终止的识别主体是操作系统，因此操作系统若能告诉父进程，子进程的终止。然后父进程可以暂时放下工作用于处理子进程的终止。

信号处理机制:  信号指特定事件发生时由操作系统向进程发送的信息

#### 10.3.2 关于JAVA的题外话: 保持开发思维

JAVA在编程语言层面支持进程或线程，但C语言及c++语言并不支持.JAVA为了保持平台移植性，以独立于操作系统的方式提供进程和线程的创建方法

#### 10.3.3 信号与signal函数

进程发现直接子进程结束时，请求操作系统调用特定函数，该请求通过调用signal函数完成(也成该函数为信号注册函数)

```c++
#include <signal.h>

void (* signal(int signo, void (*func)(int)))(int);
// 为了在产生信号时调用，返回之前注册的函数指针
```

第一个参数为特殊情况信息，第二个参数为特殊情况下将要调用的函数的地址值(指针),发生第一个参数代表的情况时，调用第二个参数所指的函数。下面给出signal中注册的部分特殊情况
- SIGALRM ： 已到通过调用alarm函数注册的时间
- SIGINT ：  输入CTRL + C
- SIGCHLD ： 子进程终止

```c++
signal(SIGCHLD,mychild);
```

信号注册后，发生注册信号时(注册的情况发生时)，操作系统将调用对应函数。

```c++
#include<unistd.h>

unsigned int alarm (unsigned int seconds);
// 返回0 或 秒为单位的距SIGALRM信号发生所剩时间
```
如果调用该函数的同时传递一个正整数型，相应时间后(以秒为单位)将产生SIGALRM信号，若传递0 ，则之前对SIGSLRM信号的预约将取消。如果通过该函数预约信号后为指定该信号对应的处理函数，则(通过调用signal)终止进程。

[signal.c](./ch10/signal.c)

```
gcc signal.c -o signal
./signal
```

上面没任何输入，下面输入了CTRL+ C

![](https://s2.ax1x.com/2019/02/09/kUpwPU.png)

我们要知道，发生信号时将唤醒由于调用sleep函数而进入阻塞状态的进程，因为进程处于睡眠状态时无法调用函数。而且，进程一旦被唤醒，就会再进入睡眠状态，即使未到sleep函数中规定的时间。


#### 10.3.4 利用sigaction 函数进行信号处理

sigaction比signal更稳定，可以取代后者。因为signal函数在UNIX系列的不同操作系统中可能存在区别，而sigaction没有不同

```c++
#include <signal.h>
// 成功时返回0，失败时返回-1 
int sigaction(int signo, const struct sigaction *act , struct sigaction *oldact);
/*
signo : 与signal函数相同，传递信号信息
act: 对于第一个参数的信号处理函数（信号处理器）信息。
oldact: 通过此参数获取之前注册的信号处理函数指针，若不需要则传递 0
*/
```
声明并初始化 sigaction 结构体变量以调用上述函数，该结构体定义如下：
```c++
struct sigaction
{
    // sa_handler 成员保存信号处理函数的指针值(地址值)
    void (*sa_handler)(int);

    // sa_mask和sa_flags的所有位默认初始化为0
    sigset_t sa_mask;
    int sa_flags;
};
```

[sigaction.c](./ch10/sigaction.c)

```
gcc sigaction.c -o sigaction
./sigaction
```
![](https://s2.ax1x.com/2019/02/09/kUkRII.png)

#### 10.3.5 利用信号处理技术消灭僵尸进程

子进程终止时将尝试SIGCHLD信号，利用这一点，我们可以完成消灭僵尸进程

[remove_zomebie.c](./ch10/remove_zomebie.c)

```
gcc remove_zomebie.c -o zombie
./zombie
```
![](https://s2.ax1x.com/2019/02/09/kUAlTA.png)

### 10.4 基于多任务的并发服务器

#### 10.4.1 基于进程的并发服务器模型

![](https://camo.githubusercontent.com/fc1ea110db4649c68d5d42335c9806295113c626/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32312f356334353336363463646532362e706e67)

可以看出，每当有客户端请求服务(连接请求)时，回声服务器端都创建子进程提供服务。步骤
- 1. 回声服务器端(父进程)通过调用accept函数受理连接请求。
- 2. 此时获取的套接字文件描述符创建并传递给子进程。
- 3. 子进程利用传递来的文件描述符提供服务

因为子进程赋值父进程拥有的所有资源，实际上不要另外经过传递文件描述符的过程

#### 10.4.2 实现并发服务器

[echo_mpserv.c](./ch10/echo_mpserv.c)

[echo_client.c](./ch10/echo_client.c)

![](https://s2.ax1x.com/2019/02/10/kUstIg.png)

服务端支持同时给多个客户端进行服务，每有一个客户端连接服务端，就会多开一个子进程，所以可以同时提供服务。

#### 10.4.3 通过fork函数复制文件描述符

fork函数时复制父进程的所有资源，但是套接字不属于进程，进程拥有的是套接字的文件描述符。

![](https://camo.githubusercontent.com/bccc953858430623a748bce77a5328554a154f58/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32312f6b5037526a782e706e67)

如图所示，一个套接字中存在两个文件描述符，只有当2个文件描述符都被销毁后，才能销毁相应套接字。

![](https://camo.githubusercontent.com/606ff8c735a23218097a755cbdcf7eb4ed0b5dbe/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32312f6b5048375a542e706e67)

### 10.5 分割TCP的I/O程序

what: 创建子进程，让父进程只负责接收数据，子进程负责写数据。这样，无论客户端是否从服务器端接收完数据都可以进行传输

![](https://camo.githubusercontent.com/2e47a1898c432b48ce0adec7c850cf783ee83564/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32312f6b5062686b442e706e67)

分割I/O程序可以提高频繁交换数据的程序的性能，如下图所示

![](https://camo.githubusercontent.com/c8ee16967cc8f5fa4f03c87b3d1cb625600cd83c/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32312f6b50627674672e706e67)

分割I/O后的客户端发送数据不必考虑接收数据的情况，因此可以连续发送数据

#### 10.5.1 回声客户端的I/O程序分割

[echo_mpclient.c](./ch10/echo_mpclient.c)

### 10.6 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于进程的说法正确的是？**

答：以下加粗的是正确的

- **从操作系统的角度上说，进程是程序运行的单位**
- **进程根据创建方式建立父子关系**
- 进程可以包含其他进程，即一个进程的内存空间可以包含其他进程
- **子进程可以创建其他子进程，而创建出来的子进程还可以创建其他子进程，但所有这些进程只与一个父进程建立父子关系。**

2. **调用 fork 函数将创建子进程，一下关于子进程正确的是？**

答：以下加粗的是正确的

- 父进程销毁时也会同时销毁子进程
- **子进程是复制父进程所有资源创建出的进程**
- 父子进程共享全局变量
- 通过 fork 函数创建的子进程将执行从开始到 fork 函数调用为止的代码。

3. **创建子进程时复制父进程所有内容，此时复制对象也包含套接字文件描述符。编写程序验证赋值的文件描述符整数值是否与原文件描述符数值相同。**

答：[text.c](./ch10/text.c)

![](https://s2.ax1x.com/2019/02/10/kU2Uvd.png)

4. **请说明进程变为僵尸进程的过程以及预防措施。**

答: 子进程完成后其实未被销毁，等到父进程销毁或着父进程主动要求得到子进程的结束状态值后，子进程才会销毁。预防措施，调用wait函数或waitid函数使父进程得到子进程的结束状态

<h2 id='1-11'>1.11 进程间通信</h2>

### 11.1 进程间通信的基本概念

进程间通信意味着两个不同进程间可以交换数据，为了完成这一点，操作系统中应该提供两个进程可以同时访问的内存空间

#### 11.1.1 对进程间通信的基本理解

无法简单实现的原因： 进程具有完全独立的内存结构，就连通过fork函数创建的子进程也不会与父进程共享内存空间。

实现： 只要有两个进程可以同时访问的内存空间，就可以通过此空间交换数据

#### 11.1.2 通过管道实现进程间通信

![](https://camo.githubusercontent.com/23dcb30d1fdfc1b0cbcc3a296af8dec7dbd59187/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32322f6b466c6b30732e706e67)

为了完成进程间通信，要创建管道，管道并非属于进程的资源而是和套接字一样属于操作系统。所以，两个进程通过操作系统提供的内存空间进行通信

```c++
#include <unistd.h>
// 成功时返回0，失败时返回-1
int pipe(int filedes[2]);
/*
filedes[0]  通过管道接收数据时使用的文件描述符， 即管道出口
filedes[1]   ...   传输数据             ...,   即管道入口   
*/
```

为了让父进程和子进程交换数据，因此需要将入口或出口中的一个文件描述符传给子进程，如何传递是通过fork函数

[pipe1.c](./ch11/pipe1.c)

```
gcc pipe1.c -o pipe1
./pipe1
```

![](https://s2.ax1x.com/2019/02/10/kUqELT.png)

可以从程序中看出，首先创建了一个管道，子进程通过 fds[1] 把数据写入管道，父进程从 fds[0] 再把数据读出来。可以从下图看出：

![](https://camo.githubusercontent.com/7aa038a10a8fb8789b75d62e1b2e6a935bfa66fd/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32322f6b46384137642e706e67)

#### 11.1.3 通过管道进行行程间双向通信

![](https://camo.githubusercontent.com/cdd0c7561c26f9519e0747d72fc7e08fbbe537ce/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32322f6b46383444652e706e67)

[pipe2.c](./ch11/pipe2.c)

![](https://s2.ax1x.com/2019/02/10/kUqhpn.png)

向管道传递数据时，先读的进程会把数据取走。所以子进程读取前会睡眠一会，等待父进程先取走数据。

当然我们可以创建两个管道，各自负责不同的数据流动即可

![](https://camo.githubusercontent.com/7f42744a48bcfa417696b0fdc7a4737e1600ff26/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32322f6b464a5730652e706e67)

使用2个管道可以避免程序流程的预测或控制

[pipe3.c](./ch11/pipe3.c)

上面通过创建两个管道实现了功能，此时，不需要额外再使用 sleep 函数。运行结果和上面一样。

### 11.2 运用进程间通信

#### 11.2.1 保存消息的回声服务器端

下面对第 10 章的 [echo_mpserv.c](./ch10/echo_mpserv.c) 进行改进，添加一个功能：
> 将回声客户端传输的字符串按序保存到文件中

实现该任务将创建一个新进程，从向客户端提供服务的进程读取字符串信息，下面是代码：

[echo_storeserv.c](./ch11/echo_storeserv.c)

![](https://s2.ax1x.com/2019/02/10/kUjFNF.png)


### 11.3 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **什么是进程间通信？分别从概念和内存的角度进行说明。**

答： 两个不同进程进行数据交换。 两个不同进行通过访问同一片内存空间，通过这片内存空间的数据变化来通信

2. **进程间通信需要特殊的 IPC 机制，这是由于操作系统提供的。进程间通信时为何需要操作系统的帮助？**

答：因为进程之间不能访问对方的内存空间，我们需要管道的帮助，而管道是由操作系统提供的让两个进程都能访问的内存空间

3. **「管道」是典型的 IPC 技法。关于管道，请回答以下问题**

+ **管道是进程间交换数据的路径。如何创建此路径？由谁创建？**

答： pipe函数，操作系统

+ **为了完成进程间通信。2 个进程要同时连接管道。那2 个进程如何连接到同一管道？**

答： 管道有两个文件描述符，两个进程每个利用一个文件描述符，父进程可以通过fork函数将文件描述符复制给子进程

+ **管道允许 2 个进程间的双向通信。双向通信中需要注意哪些内容？**

答：向管道传输数据时，数据为公共数据，先读的数据会先把数据取出，要注意读取顺序。所以我们可以用2个管道来实现双向通信

<h2 id='1-12'>1.12 I/O复用</h2>

并发服务器的第二种实现方法，基于I/O复用的服务器端构建

### 12.1 基于I/O复用的服务器端

#### 12.1.1 多进程服务器端的缺点和解决办法

创建进程时需要付出很大代价，需要大量的运算和内存空间。由于每个进程都具有独立的内存空间，所以相互间的数据交换也要求采用相对复杂的方法。

#### 12.1.2 理解复用

复用: 为了提供物理设备的效率，用最少的物理要素传递最多的数据时使用的技术

例如

![](https://camo.githubusercontent.com/d2da885b77c22fefc13eb758dfb5fb59f351dc6d/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b41384838312e706e67)

![](https://camo.githubusercontent.com/b05b0c747874b9f5f364492bcf0bd2708df3f8ae/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b41386267782e706e67)

#### 12.1.3 复用技术在服务器端的应用

![](https://camo.githubusercontent.com/03203c86a3a582f48514883546a979afa9f668c9/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b4147424d362e706e67)

![](https://camo.githubusercontent.com/d206a33c936ccd52bb967c9d9cdece20240b1ad9/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b414772714f2e706e67)

从图上可以看出，引入复用技术之后，可以减少进程数。重要的是，无论连接多少客户端，提供服务的进程只有一个。

### 12.2 理解select函数并实现服务器端

使用select函数可以将多个文件描述符集中到一起监视，项目(监视项被称为事件)如下
- 是否存在套接字接收数据
- 无需阻塞传输数据的套接字有哪些
- 哪些套接字发生了异常

select函数的调用过程:

![](https://camo.githubusercontent.com/17157a29cab47b95b75dac565fb29194fd6e948d/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b41746452732e706e67)

#### 12.2.1 设置文件描述符

select函数可以同时监视多个文件描述符即监视套接字。 将要监视的文件描述符集中，然后分为(接收，传输，异常)三类。

使用fd_set数组变量，该数组变量只存有0和1,1代表该文件描述符是监视对象。

![](https://camo.githubusercontent.com/96f1fcf063bab59f1eb72bb296a697812ea0dee2/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b41743269342e706e67)

操作：
- FD_ZERO(fd_set *fdset)：将 fd_set 变量所指的位全部初始化成0
- FD_SET(int fd,fd_set *fdset)：在参数 fdset 指向的变量中注册文件描述符 fd 的信息
- FD_SLR(int fd,fd_set *fdset)：从参数 fdset 指向的变量中清除文件描述符 fd 的信息
- FD_ISSET(int fd,fd_set *fdset)：若参数 fdset 指向的变量中包含文件描述符 fd 的信息，则返回「真」

上述函数中，FD_ISSET 用于验证 select 函数的调用结果，通过下图解释这些函数的功能：

![](https://camo.githubusercontent.com/a225b58d7bf8fbbda21429a167a88dac36f975e4/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b414e5237382e706e67)

#### 12.2.2 设置检查(监视)范围及超时

select函数
```c++
#include <sys/select.h>
#include <sys/time.h>
// 成功时返回大于0的值(发生事件的文件描述符数量)，失败时返回-1，超时返回0
int select(int maxfd,fd_set * readset, fd_set * writeset, fd_set * exceptset, const struct timeval * timeout);
/*
maxfd ：监视对象文件描述符的数量
readset: 将所以关注'是否存在待读取数据'的文件描述符注册到fd_set型变量，并传递其地址符
writeset:将所以关注'是否存在传输无阻塞数据'的文件描述符注册到fd_set型变量，并传递其地址符
exceptset: 将所以关注'是否发生异常'的文件描述符注册到fd_set型变量，并传递其地址符
timeout: 调用select函数后，为防止陷入无限阻塞的状态，传递超时信息
*/
```
**文件描述符的监视(检查)范围是?**

文件描述符的监视范围与select函数的第一个参数有关，只需将最大的文件描述符值加一在传递到select函数即可。加一是因为文件描述符的值从0开始

**如何设定select函数的超时时间？**

与最后一个参数有关.本来select函数只有监视的文件描述符发生变化时才返回，选择超时也能返回0。如果不想设置超时，则传递NULL
```
struct timeval
{
    long tv_sec;
    long tv_usec;
};
```

#### 12.2.3 调用select函数后查看结果

向 select 函数的第二到第四个参数传递的 fd_set 变量中将产生如图所示的变化：

![](https://camo.githubusercontent.com/47c7da013eabf5a3cd35c1a909dac62f333ab00a/68747470733a2f2f73322e617831782e636f6d2f323031392f30312f32332f6b41303664782e706e67)

可知，select函数调用完成后，值仍未1的位置上的文件描述符发生变化

#### 12.2.4 select函数调用示例

[select.c](./ch12/select.c)

```
gcc select.c -o select
./select
```

![](https://s2.ax1x.com/2019/02/12/kwEx7F.png)

可以看出，如果运行后在标准输入流输入数据，就会在标准输出流输出数据，但是如果 5 秒没有输入数据，就提示超时。

#### 12.2.5 实现I/O复用服务器端

[echo_selectserv.c](./ch12/echo_selectserv.c)

```
gcc echo_selectserv.c -o selserv
./selserv 9199
```

![](https://s2.ax1x.com/2019/02/12/kwZwIe.png)

### 12.3 基于windows的实现

暂略

### 12.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **请解释复用技术的通用含义，并说明何为 I/O 复用。**

答: 为了提供物理设备的效率，用最少的物理要素传递最多的数据时使用的技术.IO复用就是进程预先告诉内核需要监视的IO条件，使得内核一旦发现进程指定的一个或多个IO条件就绪，就通过进程进程处理，从而不会在单个IO上阻塞了。

[Linux网络编程-IO复用技术](https://www.cnblogs.com/luoxn28/p/6220372.html)

2. **多进程并发服务器的缺点有哪些？如何在 I/O 复用服务器中弥补？**

答：创建进程消耗大量内存与资源，I/O复用通过select函数监听套接字，通过得知是那个套接字的变化来完成对应操作

3. **复用服务器端需要 select 函数。下列关于 select 函数使用方法的描述正确的是？**

答：加粗为正确的
- 调用 select 函数前需要集中 I/O 监视对象的文件描述符
- **若已通过 select 函数注册为监视对象，则后续调用 select 函数时无需重复注册**
- 复用服务器端同一时间只能服务于 1 个客户端，因此，需要服务的客户端接入服务器端后只能等待
- **与多线程服务端不同，基于 select 的复用服务器只需要 1 个进程。因此，可以减少因创建多进程产生的服务器端的负担。**

4. **select 函数的观察对象中应包含服务端套接字（监听套接字），那么应将其包含到哪一类监听对象集合？请说明原因。**

答：应该包含到「是否存在待读取数据」

<h2>1-13 多种I/O函数</h2>

### 13.1 send&recv函数

#### 13.1.1 Linux中的send&recv

```c++
#include <sys/socket.h>
// 成功时返回发送的字节数
ssize_t send(int sockfd, const void *buf, size_t nbytes,int flags);
/*
sockfd: 表示与数据传输对象的连接的套接字文件描述符
buf   : 保存待传输数据的缓冲地址值
nbytes: 待传输的字节数
flags : 传输数据时指定的可选项信息
*/
```

```C++
#include <sys/socket.h>
// 成功时返回接收的字节数(收到EOF时返回0)，失败时返回-1
ssize_t recv(int sockfd, void *buf , size_t nbytes, int flags);
/*
sockfd: 表示与数据接收对象的连接的套接字文件描述符
buf   : 保存待接收数据的缓冲地址值
nbytes: 可接收的最大字节数
flags : 接收数据时指定的可选项信息
*/
```
send 和 recv 函数都是最后一个参数是收发数据的可选项，该选项可以用位或（bit OR）运算符（| 运算符）同时传递多个信息。

send & recv 函数的可选项意义：

可选项（Option|	    含义|	                                                          send|recv
-|-|-|-
MSG_OOB|	    用于传输带外数据（Out-of-band data|	                                    O|O
MSG_PEEK|	    验证输入缓冲中是否存在接受的数据|	                                     X|O
MSG_DONTROUTE|	数据传输过程中不参照本地路由（Routing）表，在本地（Local）网络中寻找目的地|	O|X
MSG_DONTWAIT|	调用 I/O 函数时不阻塞，用于使用非阻塞（Non-blocking）I/O|	              O|O
MSG_WAITALL|	防止函数返回，直到接收到全部请求的字节数|	                              X|O

#### 13.1.2 MSG_OOB ：发送紧急消息

MSG_OOB可选项用于传输"带外数据"紧急消息。MSG_OOB可选项用于穿甲特殊发送方法和通道以发送紧急消息

[oob_send.c](./ch13/oob_send.c)

[oob_recv.c](./ch13/oob_recv.c)

紧急消息的传输比接收简单，只需要在调用send时指定MSG_OOB可选项即可。

> - fcntl(recv_sock, F_SETOWN, getpid());
> - 文件描述符 recv_sock 指向的套接字引发的 SIGURG 信号处理进程变为 getpid 函数返回值用作 ID 进程.

因为多个进程可以共同拥有1个套接字文件。例如通过fork函数创建子进程并复制文件描述符，但是此时如果发送SIGURG信号，应该调用哪个进程的信号处理函数呢？ 显然，不可能调用所以进程的信号处理函数，因此处理SIGURG信号时必须指定处理信号的进程，而getpid函数返回调用此函数的进程ID.上述调用语句指定当前进程为处理SIGURG信号的主体。

> 通过 MSG_OOB 可选项传递数据时只返回 1 个字节，而且也不快

的确，通过 MSG_OOB 并不会加快传输速度，而通过信号处理函数 urg_handler 也只能读取一个字节。剩余数据只能通过未设置 MSG_OOB 可选项的普通输入函数读取。因为 TCP 不存在真正意义上的「外带数据」。即真正意义上的 Out-of-band 需要通过单独的通信路径高速传输数据，但是 TCP 不另外提供，只利用 TCP 的紧急模式（Urgent mode）进行传输。

#### 13.1.3 紧急模式工作原理

MSG_OOB的真正意义在于督促数据接收对象尽快处理数据，而TCP`保持顺序传输`的传输特性依然成立。

> 例如 send(sock, "890", strlen("890"), MSG_OOB);

![](https://camo.githubusercontent.com/799a2864eb80684d76f644f4b9ead6920cc7a473/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32362f356334626532323238343563632e706e67)

字符0保持于偏移量为2的位置，偏移量为3的位置存有紧急指针，紧急指针指向紧急消息的下一个位置。紧急指针指向紧急消息的下一个位置(偏移量加1)，同时向对方主机传递如下消息

> 紧急指针指向的偏移量为3之前的部分就是紧急消息

也就是说只用一个字节表示紧急消息标志

![](https://camo.githubusercontent.com/c9191b6bc40cdf023ea7b4bd6a4026ea58f2dbe4/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32362f356334626565616534366234652e706e67)

- URG=1 ：载有紧急消息的数据包
- URG指针: 紧急指针位于偏移量为3的位置

除紧急指针的前面一个字节外，数据接收方将通过调用常用输入函数读取剩余部分，所以紧急消息的意义仅在于督促消息处理

#### 13.1.4 检查输入缓冲

设置 MSG_PEEK 选项并调用 recv 函数时，即使读取了输入缓冲的数据也不会删除。因此，该选项通常与 MSG_DONTWAIT 合作，用于调用以非阻塞方式验证待读数据存与否的函数

[peek_recv.c](./ch13/peek_recv.c)

[peek_send.c](./ch13/peek_send.c)

```
gcc peek_recv.c -o recv
gcc peek_send.c -o send
./recv 9190
./send 127.0.0.1 9190
```

![](https://s2.ax1x.com/2019/02/17/kykiHP.png)

可以通过结果验证，仅发送了一次的数据被读取了 2 次，因为第一次调用 recv 函数时设置了 MSG_PEEK 可选项。

### 13.2 readv&writev函数

readv和writev函数有利于提供数据通信效率

#### 13.2.1 使用readv&writev函数

readv&writev函数功能为 `对数据进行整合传输及发送的函数`。也就是说通过writev函数可以将分散保存在第一个缓冲中的数据一并发送，通过readv函数可以由多个缓冲分别接收。因此，适当使用这两个函数有利于减少I/O 函数的调用次数

```c++
#include <sys/uio.h>
// 成功时返回发送的字节数，失败时返回-1
ssize_t writev(int filedes, const struct iovec * iov, int iovcnt);
/*
filedes : 表示数据传输对象的套接字文件描述符。也可以向read函数一样传递文件或标准输出描述符
iov     : iovec结构体数组的地址值，结构体iovec中包含待发送数据的位置和大小位置
iovcnt  : 向第二个参数传递的数组长度
*/
```
```c++
struct iovec
{
    void * iov_base;// 缓冲地址
    size_t iov_len; // 缓冲大小
}
```
![](https://camo.githubusercontent.com/79e0e4a569eb5832414595301102cbde56cf50bb/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32362f356334633631623037643230372e706e67)

writev 的第一个参数，是文件描述符，因此向控制台输出数据，ptr 是存有待发送数据信息的 iovec 数组指针。第三个参数为 2，因此，从 ptr 指向的地址开始，共浏览 2 个 iovec 结构体变量，发送这些指针指向的缓冲数据。

[writev.c](./ch13/writev.c)

```
gcc writev.c -o writev
./writevi
```

结果
```
ABC1234
Write bytes: 7
```

```c++
#include <sys/uio.h>
// 成功时返回接收的字节数，失败时返回-1
ssize_t readv(int filedes, const struct iovec *iov,int iovcnt);
/*
filedes : 传递接收数据的文件(或套接字)描述符
iov     : 包含数据保存位置和大小信息的iovec结构体数组的地址值
iovcnt  : 第二个参数中的数组的长度
*/
```

[readv.c](./ch13/readv.c)

![](https://s2.ax1x.com/2019/02/17/kyZDLd.png)

#### 13.2.2 合理使用readv&writev函数

需要传输的数据分别位于不同缓冲（数组）时，需要多次调用 write 函数。此时可通过 1 次 writev 函数调用替代操作，当然会提高效率。同样，需要将输入缓冲中的数据读入不同位置时，可以不必多次调用 read 函数，而是利用 1 次 readv 函数就能大大提高效率。

其意义在于减少数据包个数。假设为了提高效率在服务器端明确禁用了 Nagle 算法。其实 writev 函数在不采用 Nagle 算法时更有价值，如图：

![](https://camo.githubusercontent.com/e002927036b30fdd1a6b07803a9454ce988329f7/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32362f356334633733313332336531392e706e67)

### 13.3 基于windows的实现

暂略

### 13.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **下列关于 MSG_OOB 可选项的说法正确的是？**

答：加粗代表正确

- MSG_OOB 指传输 Out-of-band 数据，是通过其他路径高速传输数据
- MSG_OOB 指通过其他路径高速传输数据，因此 TCP 中设置该选项的数据先到达对方主机
- 设置 MSG_OOB 是数据先到达对方主机后，以普通数据的形式和顺序读取。也就是说，只是提高了传输速度，接收方无法识别这一点。
- **MSG_OOB 无法脱离 TCP 的默认数据传输方式，即使脱离了 MSG_OOB ，也会保持原有的传输顺序。该选项只用于要求接收方紧急处理。**

总结： MSG_OBB不会提高传输速率，只是要求接收方紧急处理

2. **利用 readv & writev 函数收发数据有何优点？分别从函数调用次数和 I/O 缓冲的角度给出说明。**

答： 减少调用次数。当要传输的数据位于不同缓冲(数组)时，wirte需要多次调用，而writev只需一次。readv也是同样道理，只需一次readv

3. **通过 recv 函数验证输入缓冲中是否存在数据时（确认后立即返回时），如何设置 recv 函数最后一个参数中的可选项？分别说明各可选项的含义**

答： 设置MSG_PEEK

<h2 id='1-14'>1.14 多播和广播</h2>

向大量客户端发送相同数据时，会对服务器端和网络流量产生负面影响。可以用多播技术解决该问题

### 14.1 多播

多播方式的数据传播是基于UDP完成的.不过多播数据同时传递到加入(注册)特定组的大量主机。换言之，多播可以同时向多个主机传递数据

#### 14.1.1 多播的数据传输方式及流量方面的优点

多播的数据传输特点
- 多播服务器针对特定多播组，只发送一次数据
- 即使只发送一次数据，但该组的所有客户端都会接收数据
- 多播组数可在IP地址范围内任意增加
- 加入特定组即可接收发往多播组的数据

多播组是D类IP地址(224.0.0.0 ~ 255.255.255.255),加入多播组相当于程序完成如下声明
> 在D类IP地址中，我希望接收发往目标239.234.218.234的多播数据

多播是基于UDP完成的，多播数据包格式与UDP数据包格式相同，不过和一般UDP数据包不同，向网络传递多播数据包时，路由器将复制该数据包并传递到多个主机。所以说多播需要借助路由器完成

![](https://camo.githubusercontent.com/dcf7b507532b4addc628966549e7d91dcc75c93f/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32372f356334643331306461613662652e706e67)

显然，多播不会向同一区域发送多个相同的数据包，这样就有利于网络流量。多播是依靠路由器复制文件并传递到主机。主要用于多媒体数据的实时传输

另外，理论上可以完成多播通信，但是不少路由器并不支持多播，或即便支持也因网络拥堵问题故意阻断多播。因此，为了在不支持多播的路由器中完成多播通信，也会使用隧道（Tunneling）技术。

#### 14.1.2 路由(Routing)和TTL(Time to Live，生存时间),及加入组的方法

为了传递多播数据包，必须设置TTL。TTL是决定数据包传递距离的主要因素，TTL用整数表示，并且每经过1个路由器就减1，TTL变为0时，该数据包无法再被传递只能销毁。因此TTL设置过大将影响网络流量，然而设置过小也会无法传递到目标

![](https://camo.githubusercontent.com/3cd1427a5573bb5f1f1a710d9bd97652ed137fa2/68747470733a2f2f692e6c6f6c692e6e65742f323031392f30312f32372f356334643339363030303165622e706e67)

TTL是通过第9章的套接字可选项完成的，设置TTL的相关协议层为IPPROTO_IP，选项名IP_MULTICAST_TTL.可以通过下面的代码把TTL设置为64

```c++
int send_sock;
int time_live = 64;
....
send_sock = socket(PF_INET,SOCK_DGRAM,0);
setsockopt(send_sock,IPPROTO_IP, IP_MULTICAST_TTL, (void*) &time_live, sizeof(time_live));
....
```

加入多播组也通过套接字选项完成，加入多播组相关的协议层为IPPROTO_IP,选项名为IP_ADD_MEMBERSHIP.通过下面的代码加入多播组

```c++
int recv_sock;
struct ip_mreq join_adr;
....
recv_sock = socket(PF_INET,SOCK_DGRAM,0);
....
join_adr.imr_multiaddr.s_addr = "//多播组地址信息";
join_adr.imr_interface.s_addr = "//加入多播组的主机地址信息";
setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
```

```c++
struct ip_mreq
{
    struct in_addr imr_multiaddr;
    struct in_addr imr_interface;
}


struct in_addr
{
    in_addr_t  s_addr; // 32位IPV4地址
}
```

#### 14.1.3 实现多播Sender (发送者) 和 Receiver (接受者)

sender是多播数据的发送主体，receiver是需要多播组加入过程的数据接收主体

sender比receiver简单，因为receiver需要经过加入组的过程，而sender只需要创建UDP套接字，并向多播地址发送数据

[news_sender.c](./ch14/news_sender.c)

[news_receiver.c](./ch14/news_receiver.c)

```
gcc news_sender.c -o sender
gcc news_receiver.c -o receiver
./sender 224.1.1.2 9190
./receiver 224.1.1.2 9190
```

![](https://s2.ax1x.com/2019/02/17/kytnFU.png)

可知，通过sender发送多播信息，receiver可以接收多播信息，如果延迟运行 receiver 将无法接受之前发送的信息。

### 14.2 广播

广播与多播类似，只有传输数据的范围有区别。多播可以跨越不同网络，只要加入同一个多播组即可，而广播只能向同一网络中的主机传输数据

#### 14.2.1 广播的理解和实现方法

广播是向同一网络中的所有主机传输数据的方法，与多播相同也是UDP传输。根据传输数据时使用的IP地址的形式，广播分为如下2种：
- 直接广播
- 本地广播

直接广播的IP地址除了网络地址外，其他主机地址全部设置为 1 。例如，希望向网络地址 192.12.34 中的所有主机传输数据时，可以向 192.12.34.255 传输。换言之，可以采取直接广播的方式向特定区域内所有主机传输数据。

而本地广播使用的IP地址限度为 255.255.255.255 ，例如，192.32.24 网络中的主机向 255.255.255.255 传输数据时，数据将传输到 192.32.24 网络中所有主机。

**数据通信中使用的IP地址是与UDP示例的唯一区别。**默认生成的套接字会阻止广播

```c++
int send_sock;
int bcast =1 ;// 对变量进行初始化以将SO_BROADCAST 选项信息改为1
....
send_sock = socket(PF_INET, SOCK_DGRAM , 0);
....
setsockopt(send_sock,SOL_SOCKET,SO_BROADCAST, (void*) & bcast, sizeof(bcast));
....
```
上述操作只需要在sender中修改，receiver的实现不需要该过程

#### 14.2.2 实现广播数据的sender和receiver

[news_sender_brd.c](./ch14/news_sender_brd.c)

[news_receiver_brd.c](./ch14/news_receiver_brd.c)

```
gcc news_receiver_brd.c -o receiver
gcc news_sender_brd.c -o sender
./sender 255.255.255.255 9190
./receiver 9190
```

![](https://s2.ax1x.com/2019/02/17/kya6Tx.png)

### 14.3 基于WIndows的实现

暂略

### 14.4 习题

> 以下答案仅代表本人个人观点，可能不是正确答案。

1. **TTL 的含义是什么？请从路由器的角度说明较大的 TTL 值与较小的 TTL 值之间的区别及问题。**

答: TTL是决定数据包传递距离的主要因素。因为每次经过1路由器，TTL减1，直到TTL为0，数据包不能被传递，只能销毁。
所以设置过大会影响网络流量，设置过小传递范围就小可能无法到达目标

2. **多播与广播的异同点是什么？请从数据通信的角度进行说明。**

答： 同 ：都是一次性向多个主机发送数据包， 异： 传输范围。多播不管在哪个网络，只要加入同一多播组，就能接收数据，而广播只能传递给某一网络中的所有主机

3. **下面关于多播的说法描述正确的是？**

答： 加错为正确

- 多播是用来加入多播组的所有主机传输数据的协议
- 主机连接到同一网络才能加入到多播组，也就是说，多播组无法跨越多个网络
- **能够加入多播组的主机数并无限制，但只能有 1个主机（Sender）向该组发送数据**
- **多播时使用的套接字是 UDP 套接字，因为多播是基于 UDP 进行数据通信的。**

4. **多播也对网络流量有利，请比较 TCP 交换方式解释其原因**

答：TCP是单对单的连接，如果想要发送给大量接收方，那么发送方要和每个接收方都进行连接，都发送相同的数据包，而多播发送方只要发送一次，一个区域只接收一次。

5. **多播方式的数据通信需要 MBone 虚拟网络。换言之，MBone 是用于多播的网络，但它是虚拟网络。请解释此处的「虚拟网络」**

答：可以理解为「通过网络中的特殊协议工作的软件概念上的网络」。也就是说， MBone 并非可以触及的物理网络。他是以物理网络为基础，通过软件方法实现的多播通信必备虚拟网络。