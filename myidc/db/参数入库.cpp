#include "_public.h"
#include "_ooci.h"
using namespace idc;


clogfile logfile;               // 本程序运行的日志。
cpactive pactive;            // 进程的心跳
cifile ifile;
//加定时器,计算时间
struct st_stcode
{
    char provname[31];  // 省
    char obtid[11];         // 站号
    char cityname[31];   // 站名
    char lat[11];             // 纬度
    char lon[11];            // 经度
    char height[11];       // 海拔高度
}stcode;
vector<struct st_stcode> stcodevet;              // 存放全国气象站点参数的容器。
bool loadstcode(const string &inifile);     // 把站点参数文件中加载到stcodelist容器中。
void EXIT(int sig);           // 程序退出和信号2、15的处理函数。

int main(int argc,char *argv[])
{
    //给出帮助文档
    if(argc<5){        
        printf("\n");
        printf("Using:./参数入库 inifile connstr charset logfile\n");

        printf("Example:/project/myidc/bin/procctl 120 /project/myidc/db/参数入库 /project/idc/ini/stcode.ini "\
                  "\"idc/idcpwd\" \"Simplified Chinese_China.AL32UTF8\" /log/idc/obtcodetodb.log\n\n");

        printf("本程序用于把全国气象站点参数数据保存到数据库的T_ZHOBTCODE表中，如果站点不存在则插入，站点已存在则更新。\n");
        printf("inifile 全国气象站点参数文件名（全路径）。\n");
        printf("connstr 数据库连接参数：username/password@tnsname\n");
        printf("charset 数据库的字符集。\n");
        printf("logfile 本程序运行的日志文件名。\n");
        printf("程序每120秒运行一次，由procctl调度。\n\n\n");return -1;
        }

    //处理信号,忽略全部信号但是打开2和15
    //closeioandsignal(true);
    signal(SIGINT,EXIT);
    signal(SIGTERM,EXIT);
    logfile.write("程序开始运行\n");
    //打开日志
    if(logfile.open(argv[4])==false)
    {
        cout<<"open log file error!\n";
        return -1;
    }
    pactive.addpinfo(10);
    //读取参数文件到对应的sql参数结构体
    if(loadstcode(argv[1])==false){return -1;}
    //stcode=stcodevet[0];
    //cout<<stcode.provname<<stcode.obtid<<endl;
    //连接数据库
    connection conn;
    if(conn.connecttodb(argv[2],argv[3])!=0){logfile.write("connect fail");printf("connect fail because %s",conn.message());return -1;}
    //创建对应的参数表
    //创建插入语句的参数表
    sqlstatement stmtinsert;
    stmtinsert.connect(&conn);
    //准备sql语句
    stmtinsert.prepare("\
        insert into T_ZHOBTCODE(obtid,cityname,provname,lat,lon,height,keyid) \
                                       values(:1,:2,:3,:4*100,:5*100,:6*10,SEQ_ZHOBTCODE.nextval)");
    //绑定sql参数
    //int obtid;
    stmtinsert.bindin(1,stcode.obtid,5);
    stmtinsert.bindin(2,stcode.cityname,30);
    stmtinsert.bindin(3,stcode.provname,30);
    stmtinsert.bindin(4,stcode.lat,10);
    stmtinsert.bindin(5,stcode.lon,10);
    stmtinsert.bindin(6,stcode.height,10);

    //创建更新语句的参数表
    sqlstatement stmtupt;
    stmtupt.connect(&conn);
    stmtupt.prepare("\
       update T_ZHOBTCODE set cityname=:1,provname=:2,lat=:3*100,lon=:4*100,height=:5*10,uptime=sysdate \
         where obtid=:6");
    stmtupt.bindin(1,stcode.cityname,30);
    stmtupt.bindin(2,stcode.provname,30);
    stmtupt.bindin(3,stcode.lat,10);
    stmtupt.bindin(4,stcode.lon,10);
    stmtupt.bindin(5,stcode.height,10);
    stmtupt.bindin(6,stcode.obtid,5);

    int insert_=0;
    int upgrate_=0;
    //执行sql语句,并且计时
    ctimer timer;

    for(auto &aa:stcodevet)
    {
        //obtid=ii+1;
        //printf("1");
        stcode=aa;
        if(stmtinsert.execute()!=0)
        {
            //printf("2");
            if(stmtinsert.rc()==1)
            {
                if(stmtupt.execute()!=0)
                {
                    logfile.write("upgrate失败,失败原因为%s\n",stmtupt.message());
                    
                }
                else upgrate_++;

            }
            else {logfile.write("execute失败,失败原因为%s\n",stmtinsert.message());}
        }
        else insert_++;
    }
    //提交事务
    conn.commit();
    logfile.write("execute success 插入了%d条数据,修改了%d条数据,耗费时间为%.2f\n",insert_,upgrate_,timer.elapsed());
    return 0;
}


// 程序退出和信号2、15的处理函数。i
void EXIT(int sig)
{
    logfile.write("程序退出,sig=%d\n\n",sig);

    exit(0);
}

bool loadstcode(const string &inifile)
{
    cifile ifile;
    if(ifile.open(inifile)==false){logfile.write("open file fail\n");return false;}
    ccmdstr cmdstr;
    string buffer;
    int i=0;
    ifile.readline(buffer);
    while(ifile.readline(buffer))
    {
        //cout<<buffer;
        memset(&stcode,0,sizeof(stcode));
    cmdstr.splittocmd(buffer,",");
    if(cmdstr.getvalue(0,stcode.provname,30)==false){logfile.write("provname failed");return false;}
    if(cmdstr.getvalue(1,stcode.obtid,5)==false){logfile.write("obtid failed");return false;}
    if(cmdstr.getvalue(2,stcode.cityname,30)==false)
    {logfile.write("cityname failed");return false;}
    if(cmdstr.getvalue(3,stcode.lat,10)==false)
    {logfile.write("lat failed");return false;}
    if(cmdstr.getvalue(4,stcode.lon,10)==false)
    {logfile.write("lon failed");return false;}
    if(cmdstr.getvalue(5,stcode.height,10)==false)
    {logfile.write("height failed");return false;}
    i++;
    
    stcodevet.push_back(stcode);
    }
    logfile.write("读取文件成功,读取了%d条\n",i);
    return true;
}

