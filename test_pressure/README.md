服务器压力测试
===============
Webbench是有名的网站压力测试工具，它是由[Lionbridge](http://www.lionbridge.com)公司开发。

> * 测试处在相同硬件上，不同服务的性能以及不同硬件上同一个服务的运行状况。
> * 展示服务器的两项内容：每秒钟响应请求数和每秒钟传输数据量。

Notice:
目前并没有实现http协议，因此对如果想要测试吞吐量的话，可以使用net_server/bin/server_echo_unittest进行测试，
1. 执行g++ webbench_mod.c -o webbench_test
2. ./server_echo_unittest
3. ./webbench_test 即可

测试规则
------------
* 测试示例

    ```C++
	webbench -c 500  -t  30   http://127.0.0.1/
    ```
* 参数

> * `-c` 表示客户端数
> * `-t` 表示时间



