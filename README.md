## Chat Room

### 项目简介

这个项目大致框架是从boost : : asio官方教程中提取出来的，并对其进行了一些改进。

https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio/example/cpp11/chat/chat_server.cpp

使用boost : : asio实现异步网络通信，I/O模型也采用了proactor模式，服务器和客户端之间传递的消息格式采用Google的protocol buffer来定义，protocol buffer也比较高效，没有采用json格式因为考虑到json的无效字符比较多，但是json的好处是通用和易懂。

### 项目结构

```bash
.
├── bin -可执行文件
│   ├── chat_room
│   └── client
├── chat_msg.h -聊天室分发消息
├── chat_room.cpp -server端
├── client.cpp -client端
├── LICENSE
├── makefile
├── Protocol.pb.cc
├── Protocol.pb.h
├── Protocol.proto -消息格式
├── README.md
├── structHeader.cpp
└── structHeader.h -消息头
```

### 运行

远程访问：

在linux下
需要安装protocol buffer
`sudo apt install protobuf-compiler`
`cd bin`
`./client 121.5.75.28 8888`

eg.
```bash
$ ./client 121.5.75.28 8888
bindname <your name>
chat <your message>
client: '' says 'hello'
'client: '' says 'hi'
'client: '' says 'hi'
'client: '' says 'lalallala'
'client: 'zww' says 'ni hao '
bindname zhr
'write message for server 5
chat welcome to chat room!
write message for server 23
client: 'zhr' says 'welcome to chat room!'
```

本地测试：

`git clone https://github.com/QuenKar/chat_room.git`

启动服务器

`./bin/chat_room <port>`

启动客户端

`./bin/client localhost <port>`

绑定姓名使用bindname，发送消息使用chat，例如：

`bindname zww`

`chat hello`