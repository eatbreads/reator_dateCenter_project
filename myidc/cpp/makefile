#开发框架的头文件路径
PUBINCL= -I/project/public

#开发框架的cpp文件路径
PUBCPP= /project/public/_public.cpp

CFLAGS= -g

all:crtsurfdata procctl 心跳测试 守护进程 清理程序 压缩程序 TCP上传 TCP下载 正向代理 webserver client client_http

crtsurfdata:生成数据进文件.cpp
	g++ -std=c++11 $(CFLAGS) -o crtsurfdata 生成数据进文件.cpp $(PUBINCL) $(PUBCPP) 
	cp -f crtsurfdata ../bin/.

procctl:调度程序.cpp
	g++ -std=c++11 $(CFLAGS) -o procctl 调度程序.cpp $(PUBINCL) $(PUBCPP) 
	cp -f procctl ../bin/.

心跳测试:心跳测试.cpp
	g++ -std=c++11 $(CFLAGS) -o 心跳测试 心跳测试.cpp $(PUBINCL) $(PUBCPP) 
	cp -f 心跳测试 ../bin/.

守护进程:守护进程.cpp
	g++ -std=c++11 $(CFLAGS) -o 守护进程 守护进程.cpp $(PUBINCL) $(PUBCPP) 
	cp -f 守护进程 ../bin/.

清理程序:清理程序.cpp
	g++ -std=c++11 $(CFLAGS) -o 清理程序 清理程序.cpp $(PUBINCL) $(PUBCPP) 
	cp -f 清理程序 ../bin/.

TCP上传:TCP上传.cpp
	g++ -std=c++11 $(CFLAGS) -o TCP上传 TCP上传.cpp $(PUBINCL) $(PUBCPP) 
	cp -f TCP上传 ../bin/.

TCP下载:TCP下载.cpp
	g++ -std=c++11 $(CFLAGS) -o TCP下载 TCP下载.cpp $(PUBINCL) $(PUBCPP) 
	cp -f TCP下载 ../bin/.
正向代理:正向代理1.cpp
	g++ -std=c++11 $(CFLAGS) -o 正向代理 正向代理1.cpp $(PUBINCL) $(PUBCPP) 
	cp -f 正向代理 ../bin/.
webserver:webserver.cpp
	g++ -std=c++11 $(CFLAGS) -o webserver webserver.cpp $(PUBINCL) $(PUBCPP) -pthread
	cp -f webserver ../bin/.
client:client.cpp
	g++ -std=c++11 $(CFLAGS) -o client client.cpp $(PUBINCL) $(PUBCPP) 
	cp -f client ../bin/.
client_http:client_http.cpp
	g++ -std=c++11 $(CFLAGS) -o client_http client_http.cpp $(PUBINCL) $(PUBCPP) 
	cp -f client_http ../bin/.
clean:
	rm -f crtsurfdata procctl