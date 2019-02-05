## 套接字的多种可选项

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

[sock_type.c](./sock_type.c)

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

[get_buf.c](./get_buf.c)

编译运行
```
gcc get_buf.c -o getbuf
./getbuf
```

![](https://s2.ax1x.com/2019/02/05/kJxEzq.png)

下面的程序要更改I/O缓冲大小

[set_buf.c](./set_buf.c)

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

[reuseadr_eserver.c](./reuseadr_eserver.c)

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