## 实现基于UDP的服务器端/客户端

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

[uecho_client.c](./uecho_client.c)

[uecho_server.c](./uecho_server.c)

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

[bound_host1.c](./bound_host1.c)

[bound_host2.c](./bound_host2.c)

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

将之前示例的[uecho_client.c](./uecho_client.c)程序改成基于UDP套接字的程序，并结合之前的[uecho_server.c](./uecho_server.c)运行

[uecho_con_client.c](./uecho_con_client.c)

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

