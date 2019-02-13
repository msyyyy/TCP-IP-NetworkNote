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

紧急消息的传输比接收简单，只需要在调用send时指定MSG_OOB可选项即可