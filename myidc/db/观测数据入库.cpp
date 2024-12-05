#include "_public.h"
#include "_ooci.h"
using namespace idc;


clogfile logfile;               // 本程序运行的日志。
cpactive pactive;            // 进程的心跳

//加定时器,计算时间
struct st_zhobtmind
{
    char obtid[6];            // 站点代码。
    char ddatetime[21];  // 数据时间，精确到分钟。
    char t[11];                 // 温度，单位：0.1摄氏度。
    char p[11];                // 气压，单位：0.1百帕。
    char u[11];                // 相对湿度，0-100之间的值。
    char wd[11];             // 风向，0-360之间的值。
    char wf[11];              // 风速：单位0.1m/s。
    char r[11];                // 降雨量：0.1mm。
    char vis[11];             // 能见度：0.1米。
}stzhobtmind;
// 业务处理主函数。
bool _obtmindtodb(const char *pathname,const char *connstr,const char *charset);
//保存临时的文件名的数组
vector<string> stringvet;
//执行次数
int ti=0;
void EXIT(int sig);           // 程序退出和信号2、15的处理函数。
int main(int argc,char *argv[])
{
    // 帮助文档。
    if (argc!=5)
    {
        printf("\n");
        printf("Using:./观测数据入库 pathname connstr charset logfile\n");

        printf("Example:/project/myidc/bin/procctl 10 /project/myidc/db/观测数据入库 /tmp/idc/surfdata "\
                  "\"idc/idcpwd\" \"Simplified Chinese_China.AL32UTF8\" /log/idc/obtmindtodb.log\n\n");

        printf("本程序用于把全国气象观测数据文件入库到T_ZHOBTMIND表中，支持xml和csv两种文件格式，数据只插入，不更新。\n");
        printf("pathname 全国气象观测数据文件存放的目录。\n");
        printf("connstr  数据库连接参数：username/password@tnsname\n");
        printf("charset  数据库的字符集。\n");
        printf("logfile  本程序运行的日志文件名。\n");
        printf("程序每10秒运行一次，由procctl调度。\n\n\n");

        return -1;
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
    pactive.addpinfo(100);
    ctimer timer;
    
    //执行业务处理函数
    if(_obtmindtodb(argv[1],argv[2],argv[3])==false){logfile.write("execute fail\n"); return -1;}
    logfile.write("execute success 插入了%d条数据,耗费时间为%.2f\n",ti,timer.elapsed());
    return 0;
}


// 程序退出和信号2、15的处理函数。i
void EXIT(int sig)
{
    logfile.write("程序退出,sig=%d\n\n",sig);

    exit(0);
}
bool _obtmindtodb(const char *pathname,const char *connstr,const char *charset)
{
     connection conn;
    if(conn.connecttodb(connstr,charset)!=0){logfile.write("connect fail");printf("connect fail because %s",conn.message());return false;}
    //创建插入语句的参数表
    //绑定结构体到对应的sql语句
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare("insert into T_ZHOBTMIND(obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid) "\
                        "values(:1,to_date(:2,'yyyymmddhh24miss'),:3,:4,:5,:6,:7,:8,:9,SEQ_ZHOBTMIND.nextval)");
    stmt.bindin(1,stzhobtmind.obtid,5);
    stmt.bindin(2,stzhobtmind.ddatetime,14);
    stmt.bindin(3,stzhobtmind.t,10);
    stmt.bindin(4,stzhobtmind.p,10);
    stmt.bindin(5,stzhobtmind.u,10);
    stmt.bindin(6,stzhobtmind.wd,10);
    stmt.bindin(7,stzhobtmind.wf,10);
    stmt.bindin(8,stzhobtmind.r,10);
    stmt.bindin(9,stzhobtmind.vis,10);
    
    

    //打开目标目录
    cdir dir;
    if(dir.opendir(pathname,"*.xml")==false){logfile.write("opendir error\n");return false;};
    //拿到目标目录中的文件名列表
    while(dir.readdir())
    {
        stringvet.push_back(dir.m_ffilename);
    }
    //遍历这个列表依次打开文件
     //执行sql语句,并且计时
    
    for(auto &a:stringvet)
    {
        //读取文件并且解析xml格式进结构体
        cifile ifile;
        if(ifile.open(a)==false){logfile.write("open failed\n");return false;}
        string temp;
        while(ifile.readline(temp))
        {
            //解析内容到结构体
            memset(&stzhobtmind, 0, sizeof(stzhobtmind));
            if(getxmlbuffer(temp,"obtid",stzhobtmind.obtid,5)==false){logfile.write("obtid fail\n");return false;}
            if(getxmlbuffer(temp,"ddatetime",stzhobtmind.ddatetime, 14)==false){logfile.write("ddatetime fail\n");return false;}
            if(getxmlbuffer(temp,"t",stzhobtmind.t,10)==false){logfile.write("t fail\n");return false;}
            if(getxmlbuffer(temp,"p",stzhobtmind.p, 10)==false){logfile.write("p fail\n");return false;}
            if(getxmlbuffer(temp,"u",stzhobtmind.u, 10)==false){logfile.write("u fail\n");return false;}
            if(getxmlbuffer(temp,"wd",stzhobtmind.wd, 10)==false){logfile.write("wd fail\n");return false;}
            if(getxmlbuffer(temp,"wf",stzhobtmind.wf, 10)==false){logfile.write("wf fail\n");return false;}
            if(getxmlbuffer(temp,"r",stzhobtmind.r, 10)==false){logfile.write("r fail\n");return false;}
            if(getxmlbuffer(temp,"vis",stzhobtmind.vis, 10)==false){logfile.write("vis fail\n");return false;}
            //执行语句
            if(stmt.execute()!=0){logfile.write("execute fail:%s\n",stmt.message());}
            else
            {
                ti++;
            }
        }
        //提交事务
        conn.commit();

    }
    return true;
}

