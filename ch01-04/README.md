- [开始网络编程](#1)
    - [理解网络编程和套接字](#1-1)
    - [套接字类型与协议设置](#1-2)
    - [地址族与数据序列](#1-3)
    - [基于TCP的服务器端/客户端(1)](#1-4)


<h1 id='1'>开始网络编程</h1>

<h2 id='1-1'>理解网络编程和套接字</h2>

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

<h2 id='1-2'>套接字类型与协议设置</h2>

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

<h2 id='1-3'>地址族与数据序列</h2>

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


<h2 id='1-4'>基于TCP的服务器端/客户端(1)</h2>

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

[迭代回声服务器端](./echo_server.c)

[迭代回声客户端](./echo_client.c)

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
