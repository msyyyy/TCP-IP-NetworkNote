## 8 域名及网络地址(DNS)

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

[gethostbyname.c](./gethostbyname.c)

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

[gethostbyaddr.c](./gethostbyaddr.c)

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