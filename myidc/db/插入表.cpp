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
    struct st_girl
     {
        long id;                 // 超女编号，用long数据类型对应Oracle无小数的number(10)。
        char name[31];     // 超女姓名，用char[31]对应Oracle的varchar2(30)。
        double weight;     // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
        char btime[20];     // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ssi'。
        char memo[301];  // 备注，用char[301]对应Oracle的varchar2(300)。
    } stgirl;
    stmt.prepare("insert into my_table(id,myname,weight,btime,memo) \
                    values(:1,:2,:3,to_date(:4,'yyyy-mm-dd hh24:mi:ss'),:5)");
    stmt.bindin(1,stgirl.id);
    stmt.bindin(2,stgirl.name,30);
    stmt.bindin(3,stgirl.weight);
    stmt.bindin(4,stgirl.btime,19);
    stmt.bindin(5,stgirl.memo,300);

    for(int ii=30;ii<40;ii++)
    {
        memset(&stgirl,0,sizeof(stgirl));
        // 为变量赋值。
        stgirl.id=ii;                                                                                 // 超女编号。
        sprintf(stgirl.name,"西施%05dgirl",ii);                                       // 超女姓名。
        stgirl.weight=45.35+ii;      
        //to_date(:4,'yyyy-mm-dd hh24:mi:ss')                                                        // 超女体重。
        sprintf(stgirl.btime,"2021-08-25 10:33:%02d",ii);                      // 报名时间。
        sprintf(stgirl.memo,"这是'第%05d个超级女生的备注。",ii);         // 备注。
        if(stmt.execute()!=0){printf("execute fail\n");}
    }
    
    //printf("execute seccess\n");
    conn.commit();
    return 0;
}