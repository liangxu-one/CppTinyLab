# EpollReactor
1. mkdir build && make bin, 进入build执行cmake ..
2. 在build下执行make, 生成可执行文件, 在bin目录下 
3. 运行./main ip port, 其中ip, port分别代表ip地址和端口号, 例如./main 0.0.0.0 9999 
4. 使用nc进行测试, nc ip port, 其中ip为运行./main所在服务器的ip地址, port为运行./main时指定的端口号