#########################################
# 请注意，编译时，确定以下几点：
# 1、已安装oracle的客户端或oracle服务端程序，并且必须安装好OCI软件包
# 2、设置好ORACLE_HOME环境变量
#########################################

# oracle头文件路径
ORAINCL = -I/oracle/home/rdbms/public

# oracle库文件路径
ORALIB =  -L/oracle/home/lib -L.

# oracle的oci库
ORALIBS = -lclntsh

#CFLAGS = -O2
#CFLAGS = -O2 -Wall
CFLAGS = -g -Wall -Wno-write-strings #-Wno-unused-variable

#开发框架的头文件路径
PUBINCL= -I/project/public

#开发框架的cpp文件路径
PUBCPP= /project/public/_public.cpp

#tool的头文件和cpp文件路径
MYPUBINCL= -I/project/tools/cpp
MYPUBCPP= /project/tools/cpp/_tools.cpp

all:创建表 插入表 查询表 参数入库 观测数据入库 数据抽取 xml入库

创建表: 创建表.cpp _ooci.h _ooci.cpp 
	g++ -std=c++11 $(CFLAGS) -o 创建表  创建表.cpp $(ORAINCL) $(ORALIB) $(ORALIBS)  _ooci.cpp
插入表: 插入表.cpp _ooci.h _ooci.cpp 
	g++ -std=c++11 $(CFLAGS) -o 插入表  插入表.cpp $(ORAINCL) $(ORALIB) $(ORALIBS)  _ooci.cp
查询表: 查询表.cpp _ooci.h _ooci.cpp 
	g++ -std=c++11 $(CFLAGS) -o 查询表  查询表.cpp $(ORAINCL) $(ORALIB) $(ORALIBS)  _ooci.cpp
参数入库: 参数入库.cpp _ooci.h _ooci.cpp 
	g++ -std=c++11 -g -o 参数入库  参数入库.cpp $(ORAINCL) $(ORALIB) $(ORALIBS)  _ooci.cpp $(PUBINCL) $(PUBCPP)
观测数据入库: 观测数据入库.cpp _ooci.h _ooci.cpp 
	g++ -std=c++11 -g -o 观测数据入库  观测数据入库.cpp $(ORAINCL) $(ORALIB) $(ORALIBS)  _ooci.cpp $(PUBINCL) $(PUBCPP)
数据抽取: 数据抽取.cpp _ooci.h _ooci.cpp 
	g++ -std=c++11 -g -o 数据抽取  数据抽取.cpp $(ORAINCL) $(ORALIB) $(ORALIBS)  _ooci.cpp $(PUBINCL) $(PUBCPP)
xml入库: xml入库.cpp _ooci.h _ooci.cpp _tools.h _tools.cpp
	g++ -std=c++11 -g -o xml入库  xml入库.cpp $(ORAINCL) $(ORALIB) $(ORALIBS)  _ooci.cpp _tools.cpp $(PUBINCL) $(PUBCPP) 
clean:
	rm -rf 创建表 

