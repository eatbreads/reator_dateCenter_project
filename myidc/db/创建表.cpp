#include "_ooci.h"   // 开发框架操作Oracle的头文件。
#include<iostream>
using namespace idc;
using namespace std;

int main(int argc, char* argv[])
{
    //创建连接
    connection conn;
    
    //连接数据库
    if(conn.connecttodb("scott/123456","Simplified Chinese_China.AL32UTF8")!=0)
    {
        printf("connect database failed.\n%s\n",conn.message());return -1;
    }
    //创建语句
    sqlstatement stmt;
    //绑定到数据库
    if(stmt.connect(&conn)!=0){printf("connect database failed");return -1;}

    //准备语句
    stmt.prepare("\
       create table my_table2(id    number(10),\
                                    myname  varchar2(30),\
                                    weight   number(8,2),\
                                    btime date,\
                                    memo  varchar2(300),\
                                    pic   blob,\
                                    primary key (id))"); 

    //执行语句
    if(stmt.execute()!=0){printf("Error execute\n"); return -1;}


    return 0;
}