## 17 优于select的epoll

实现I/O复用的传统方法有select函数和poll函数，但是在LINUX下有性能更好的epoll可以选择

### 17.1 epoll 理解及应用

select无论如何优化程序性能都无法同时接入上百个客户端。这种select方式并不适合以web服务器端开发为主流的现代开发环境，所以要学习epoll

#### 17.1.1 基于select的I/O复用技术速度慢的原因

主要有以下两点

- 调用select函数后常见的针对所以文件描述符的循环语句
- 每次调用select函数时都需要向该函数传递监视对象信息

因为select不是把所以发生变化的文件描述符单独集中起来，而是观察监视对象fd_set的变化找出发生变换的文件描述符，所以要遍历所有的文件描述符，而且为了观察是否发生变换,在select函数之前要复制并保存原有信息，以便于对照，并在每次调用select函数时传递新的监视对象信息

最大的性能负担是由于每次传递监视对象信息即 **每次调用select函数时向操作系统传递监视对象信息**。因为应用程序向操作系统传递数据对程序是很大的负担，而且无法优化，那么我们为什么要把监视对象传递给操作系统呢？ 

因为select函数与文件描述符有关，更准确的说是监视套接字变换的函数，而套接字是有操作系统管理的，所以select需要借助操作系统完成。select函数的这一缺点可以这样弥补(Linux下弥补的方法为epoll，windows是IOCP)

> 仅向操作系统传递一次监视对象，监视范围或内容变化时只通知发生变化的事项

#### 17.1.2 select也有优点

最主要的就是select具有兼容性，适用于
- 服务器端接入者少
- 程序应具有兼容性

#### 17.1.3 实现epoll时必要的函数和结构体

epoll的优点
- 无需编写以监视状态变化为目的的针对所有文件描述符的循环语句
- 调用对应于select函数的epoll_wait函数时无需每次传递监视对象信息

epoll服务器实现中需要的3个函数
```
epoll_create : 创建保存epoll文件描述符的空间
epoll_ctl    : 向空间注册并注销文件描述符
epoll_wait   : 与select函数类似，等待文件描述符发生变化
```

目的| select | epoll 
-|-|-
保存监视对象文件描述符 |  声明fd_set变量  | epoll下由操作系统负责保存监视对象文件描述符，需要向操作系统请求创建保存文件描述符的空间 ,epoll_create
添加和删除监视对象文件描述符| FD_SET , FD_CLR函数 | epoll_ctl函数请求操作系统完成
等待文件描述符的变化| select函数 | epoll_wait函数
查看监视对象的状态变换| fd_set变量 | epoll_event结构体

```c++
struct epoll_event
{
     __uint32_t events;
     epoll_data_t data;
}

typedef union epoll_data
{
    void * ptr;
    int fd;
    __uint32_t u32;
    __uint64_t u64;
} epoll_data_t;
```

声明足够大的epoll_event结构体数组后，传递给epoll_wait函数，发生变化的文件描述符信息将被填入该数组。因此，无需像select函数那样针对所有文件描述符进行循环

#### 17.1.4 epoll_create

epoll 是从 Linux 的 2.5.44 版内核开始引入的。通过以下命令可以查看 Linux 内核版本：

> cat /proc/sys/kernel/osrelease

```c++
#include <sys/epoll.h>
// 成功时返回epoll文件描述符，失败时返回-1
int epoll_create(int size);
/*
size : epoll实例的大小，仅供操作系统参考
*/
```

> Linuix 2.6.8之后的内核将完全忽略传递的size参数

epoll_create函数创建的资源和套接字相同，也有操作系统管理。返回的文件描述符主要用于区分epoll例程，需要终止时，调用close

#### 17.1.5 epoll_ctl

生成例程后，应在其内部注册监视对象文件描述符，此时使用epoll_ctl函数

```c++
#include <sys/epoll.h>
// 成功时返回0，失败时返回-1
int epoll_ctl (int epfd, int op, int fd, struct epoll_event * event);
/*
epfd  : 用于注册监视对象的epoll例程的文件描述符
op    ：用于指定监视对象的添加，删除等更改操作
fd    : 需要注册的监视对象文件描述符
event : 监视对象的事件类型
*/
```
```
例如 epoll_ctl (A, EPOLL_CTL_ADD, B ,C )

epoll例程A中注册文件描述符B，主要目的是监视参数C中的数据

epoll_ctl (A, EPOLL_CTL_DEL , B, NULL)

从epoll例程A中删除文件描述符B (删除对象时不需要事件类型)
```

- EPOLL_CTL_ADD：将文件描述符注册到 epoll 例程
- EPOLL_CTL_DEL：从 epoll 例程中删除文件描述符
- EPOLL_CTL_MOD：更改注册的文件描述符的关注事件发生情况

epoll_event 可以在注册文件描述符时，用于注册关注的事件

```c++
struct epoll_event event;
...
event.events=EPOLLIN;//发生需要读取数据的情况时
event.data.fd=sockfd;
epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event);
...
```
上述代码将 epfd 注册到 epoll 例程 epfd 中，并在需要读取数据的情况下产生相应事件。接下来给出 epoll_event 的成员 events 中可以保存的常量及所指的事件类型。

- EPOLLIN：需要读取数据的情况
- EPOLLOUT：输出缓冲为空，可以立即发送数据的情况
- EPOLLPRI：收到 OOB 数据的情况
- EPOLLRDHUP：断开连接或半关闭的情况，这在边缘触发方式下非常有用
- EPOLLERR：发生错误的情况
- EPOLLET：以边缘触发的方式得到事件通知
- EPOLLONESHOT：发生一次事件后，相应文件描述符不再收到事件通知。因此需要向 epoll_ctl 函数的第二个参数传递EPOLL_CTL_MOD ，再次设置事件。

可通过位运算同事传递多个上述参数。

#### 17.1.6 epoll_wait 

```C++
#include <sys/epoll.h>
// 成功时返回事件的文件描述符数，失败时返回-1
int epoll_wait (int epfd , struct epoll_event * events , int maxevents , int timeout);
/*
epfd   : 表示事件发送监视范围内的epoll例程的文件描述符
events ： 保存发生事件的文件描述符集合的结构体地址值
maxevents : 第二个参数可以保存的最大事件数
timeout  : 以1/10000秒为单位的等待时间，传递-1 时，一直等待直到事件发生
*/
```
需要注意的是，第二个参数需要动态分配
```
int event_cnt;
struct epoll_event *ep_events;
...
ep_events=malloc(sizeof(struct epoll_event)*EPOLL_SIZE);//EPOLL_SIZE是宏常量
...
event_cnt=epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);
...
```
调用函数后，返回发生事件的文件描述符，同时在第二个参数指向的缓冲中保存发生事件的文件描述符集合。因此，无需像 select 一样插入针对所有文件描述符的循环。

#### 17.1.7 基于epoll的回声服务器

