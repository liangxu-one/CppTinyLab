# MyEpollReactor
1. 执行make libevent.a文件先生成静态库文件 
2. 执行make main文件生成可执行文件 
3. 运行./main ip port, 其中ip, port分别代表ip地址和端口号, 例如./main 0.0.0.0 9999 
4. 使用nc进行测试, nc ip port, 其中ip为运行./main所在服务器的ip地址, port为运行./main时指定的端口号