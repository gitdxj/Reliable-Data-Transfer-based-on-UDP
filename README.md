# Reliable-Data-Transfer-based-on-UDP
对UDP封装，实现可靠传输。
定时重传部分有差错，当时为了唬助教就随便写了写（反正我又没法给他演示超时）。
WinSock2的udp recvfrom函数是阻塞的，定时重传应当设置阻塞时间。我代码里的实现方法是不正确的。
我写的rdtSocket以自顶向下里的rdt3.0为模板，这里只不过把下层的unreliable channel换成了udp。
