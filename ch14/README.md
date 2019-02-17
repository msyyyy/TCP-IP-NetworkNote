## 14 多播和广播

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

[news_sender.c](./news_sender.c)

[news_receiver.c](./news_receiver.c)

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

[news_sender_brd.c](./news_sender_brd.c)

[news_receiver_brd.c](./news_receiver_brd.c)

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