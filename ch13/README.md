## 13 多种I/O函数

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

[oob_send.c](./oob_send.c)

[oob_recv.c](./oob_recv.c)

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

[peek_recv.c](./peek_recv.c)

[peek_send.c](./peek_send.c)

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

[writev.c](./writev.c)

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

[readv.c](./readv.c)

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