帮我使用httplib完成下列要求
1、通过setting路由指定服务器模式，是client还是server模式，如果是server模式则监听8848端口，如果是client模式则不监听，反而需要通过destination路由指定server的地址和端口
2、成功建立连接后需要两个服务器间进行密钥交换，并可以通过status路由查询状态
3、最后通过webs0ocket协议或别的也行，在/chat路由进行双向的加密通信（使用messageencryptor）
4、最好能给出一个测试用的html, 直接使用httplib作为http服务器，运行在1145端口
5、所有新增文件放置在end2end目录下
6、使用encrypter/encrypter.cpp完成密钥交换和明文加密功能
7、不要更改任何已存在的代码，新的代码要写在/server目录下
8、需要输出各服务的端口号，如果当前端口不可用则尝试+1，最大次数为10

数据交换结构如下

setting路由
{
    "mode": "server" | "client",
    "destination": {
        "host": "localhost",
        "port": 8848
    } // 仅在client模式下需要
}

status路由
{
    "status": "connected" | "disconnected",
}