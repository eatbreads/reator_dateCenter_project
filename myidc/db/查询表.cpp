#include"_ooci.h"
#include<iostream>
using namespace std;
using namespace idc;


int main(int args,char* argv[])
{
    connection conn;
    if(conn.connecttodb("scott/123456","Simplified Chinese_China.AL32UTF8")!=0)
    {
        printf("connection fail\n");printf("%s",conn.message());return -1;
    }
    sqlstatement stmt;
    if(stmt.connect(&conn)!=0){printf("sql statement fail\n");return -1;}
    //准备容器和对应的数据
    int minid=11,maxid=13;
    struct st_girl
     {
        long id;                 // 超女编号，用long数据类型对应Oracle无小数的number(10)。
        char name[31];     // 超女姓名，用char[31]对应Oracle的varchar2(30)。
        double weight;     // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
        char btime[20];     // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ssi'。
        char memo[301];  // 备注，用char[301]对应Oracle的varchar2(300)。
    } stgirl;

    //准备sql语句
    stmt.prepare("select id,myname,weight,to_char(btime,'yyyy-mm-dd hh24:mi:ss'),memo from my_table where id>=11 and id<=13");

    //进行容器和数据的绑定 ,和输出的重定向
    stmt.bindin(1,minid);
    stmt.bindin(2,maxid);
    stmt.bindout(1,stgirl.id);
    stmt.bindout(2,stgirl.name,30);
    stmt.bindout(3,stgirl.weight);
    stmt.bindout(4,stgirl.btime,19);
    stmt.bindout(5,stgirl.memo,300);
    //执行语句
    if(stmt.execute()!=0){printf("execute fail(%s)\n",stmt.message());}    

    //从结果集拿到记录
    while(true)
    {
        memset(&stgirl,0,sizeof(stgirl));
        if(stmt.next()!=0)break;
        printf("id=%ld,myname=%s,weight=%.02f,btime=%s,memo=%s\n",stgirl.id,stgirl.name,stgirl.weight,stgirl.btime,stgirl.memo);

    }
    // 请注意，stmt.m_cda.rpc变量非常重要，它保存了SQL被执行后影响的记录数。
    printf("本次查询了my_table表%ld条记录。\n",stmt.rpc());
    return 0;
}