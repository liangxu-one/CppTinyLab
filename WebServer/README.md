# WebServer
1. mkdir build && make bin, 进入build执行cmake .. 
2. 在build下执行make, 生成可执行文件, 在bin目录下 
3. 运行./main ip port, 其中ip, port分别代表ip地址和端口号, 例如./main 0.0.0.0 9999 
4. 浏览器输入ip:port进行访问, 目前文件工作目录为webpath, 因此默认打开此目录下的详细内容<br>
(该项目根据黑马WebServer项目编写, 与其源代码稍有不同, 目前存在部分bug, 当文件描述符设置为非阻塞时, 无法成功传输大文件)