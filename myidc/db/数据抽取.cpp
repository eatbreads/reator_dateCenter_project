#include "_public.h"
#include "_ooci.h"
using namespace idc;


clogfile logfile;               // 本程序运行的日志。
cpactive pactive;            // 进程的心跳
connection conn;            // 数据源数据库。
// 程序运行参数的结构体。
struct st_arg
{
    char connstr[101];       // 数据库的连接参数。
    char charset[51];         // 数据库的字符集。
    char selectsql[1024];   // 从数据源数据库抽取数据的SQL语句。
    char fieldstr[501];        // 抽取数据的SQL语句输出结果集字段名，字段名之间用逗号分隔。
    char fieldlen[501];       // 抽取数据的SQL语句输出结果集字段的长度，用逗号分隔。
    char bfilename[31];     // 输出xml文件的前缀。
    char efilename[31];     // 输出xml文件的后缀。
    char outpath[256];      // 输出xml文件存放的目录。
    int    maxcount;           // 输出xml文件最大记录数，0表示无限制。
    char starttime[52];      // 程序运行的时间区间
    char incfield[31];         // 递增字段名。
    char incfilename[256]; // 已抽取数据的递增字段最大值存放的文件。
    char connstr1[101];     // 已抽取数据的递增字段最大值存放的数据库的连接参数。
    int    timeout;              // 进程心跳的超时时间。
    char pname[51];          // 进程名，建议用"dminingoracle_后缀"的方式。
} starg;
//结果集字段名数组和长度数组
ccmdstr fieldname; 
ccmdstr fieldlen;
// 把xml解析到参数starg结构中。
bool xmltoarg(const char *strxmlbuffer);
//保存临时的文件名的数组
void help();
// 判断当前时间是否在程序运行的时间区间内。
bool instarttime();
// 数据抽取的主函数。
bool dminingoracle();
//执行次数
int ti;
ctimer timer;
void EXIT(int sig);           // 程序退出和信号2、15的处理函数。
int main(int argc,char *argv[])
{
    // 帮助文档。
    if (argc!=3){help();return -1;}
    //查询结果上传本地
    //处理信号,忽略全部信号但是打开2和15
    //closeioandsignal(true);
    signal(SIGINT,EXIT);
    signal(SIGTERM,EXIT);
    //打开日志
    if(logfile.open(argv[1])==false)
    {
        cout<<"open log file error!\n";
        return -1;
    }
    //解析xml获得参数
    if(xmltoarg(argv[2])==false){logfile.write("解析参数失败\n");return -1;}
    // 判断当前时间是否在程序运行的时间区间内。
    if (instarttime()==false) return 0;
    // 连接数据源的数据库。
    if (conn.connecttodb(starg.connstr,starg.charset)!=0)
    {
        logfile.write("connect database(%s) failed.\n%s\n",starg.connstr,conn.message()); EXIT(-1);
    }
    logfile.write("connect database(%s) ok.\n",starg.connstr);
    //执行业务处理函数
    if(dminingoracle()==false){logfile.write("业务处理失败\n");return -1;}

    //logfile.write("execute success 插入了%d条数据,耗费时间为%.2f\n",ti,timer.elapsed());
    return 0;
}
// 程序退出和信号2、15的处理函数。i
void EXIT(int sig)
{
    logfile.write("程序退出,sig=%d\n\n",sig);

    exit(0);
}
void help()
{
    printf("Using:/project/tools/bin/数据抽取 logfilename xmlbuffer\n\n");

    printf("Sample:/project/myidc/bin/procctl 3600 /project/myidc/db/数据抽取 /log/idc/dminingoracle_ZHOBTCODE.log "
              "\"<connstr>idc/idcpwd</connstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<selectsql>select obtid,cityname,provname,lat,lon,height from T_ZHOBTCODE where obtid like '5%%%%'</selectsql>"\
              "<fieldstr>obtid,cityname,provname,lat,lon,height</fieldstr><fieldlen>5,30,30,10,10,10</fieldlen>"\
              "<bfilename>ZHOBTCODE</bfilename><efilename>togxpt</efilename><outpath>/idcdata/dmindata</outpath>"\
              "<timeout>30</timeout><pname>dminingoracle_ZHOBTCODE</pname>\"\n\n");
    printf("       /project/tools/bin/procctl   30 /project/tools/bin/dminingoracle /log/idc/dminingoracle_ZHOBTMIND.log "\
              "\"<connstr>idc/idcpwd@snorcl11g_128</connstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<selectsql>select obtid,to_char(ddatetime,'yyyymmddhh24miss'),t,p,u,wd,wf,r,vis,keyid from T_ZHOBTMIND where keyid>:1 and obtid like '5%%%%'</selectsql>"\
              "<fieldstr>obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid</fieldstr><fieldlen>5,19,8,8,8,8,8,8,8,15</fieldlen>"\
              "<bfilename>ZHOBTMIND</bfilename><efilename>togxpt</efilename><outpath>/idcdata/dmindata</outpath>"\
              "<starttime></starttime><incfield>keyid</incfield>"\
              "<incfilename>/idcdata/dmining/dminingoracle_ZHOBTMIND_togxpt.keyid</incfilename>"\
              "<timeout>30</timeout><pname>dminingoracle_ZHOBTMIND_togxpt</pname>"\
              "<maxcount>1000</maxcount><connstr1>scott/tiger@snorcl11g_128</connstr1>\"\n\n");

    printf("本程序是共享平台的公共功能模块，用于从Oracle数据库源表抽取数据，生成xml文件。\n");
    printf("logfilename 本程序运行的日志文件。\n");
    printf("xmlbuffer   本程序运行的参数，用xml表示，具体如下：\n\n");

    printf("connstr     数据源数据库的连接参数，格式：username/passwd@tnsname。\n");
    printf("charset     数据库的字符集，这个参数要与数据源数据库保持一致，否则会出现中文乱码的情况。\n");
    printf("selectsql   从数据源数据库抽取数据的SQL语句，如果是增量抽取，一定要用递增字段作为查询条件，如where keyid>:1。\n");
    printf("fieldstr    抽取数据的SQL语句输出结果集的字段名列表，中间用逗号分隔，将作为xml文件的字段名。\n");
    printf("fieldlen    抽取数据的SQL语句输出结果集字段的长度列表，中间用逗号分隔。fieldstr与fieldlen的字段必须一一对应。\n");
    printf("bfilename   输出xml文件的前缀。\n");
    printf("efilename   输出xml文件的后缀。\n"); 
    printf("outpath     输出xml文件存放的目录。\n");
    printf("timeout     本程序的超时时间，单位：秒。\n");
    printf("pname       进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n\n");

    printf("starttime   程序运行的时间区间，例如02,13表示：如果程序启动时，踏中02时和13时则运行，其它时间不运行。"\
         "如果starttime为空，表示不启用，只要本程序启动，就会执行数据抽取任务，为了减少数据源数据库压力"\
         "抽取数据的时候，如果对时效性没有要求，一般在数据源数据库空闲的时候时进行。\n");
    printf("maxcount    输出xml文件的最大记录数，缺省是0，表示无限制，如果本参数取值为0，注意适当加大timeout的取值，防止程序超时。\n");
    printf("incfield    递增字段名，它必须是fieldstr中的字段名，并且只能是整型，一般为自增字段。"\
          "如果incfield为空，表示不采用增量抽取的方案。");
    printf("incfilename 已抽取数据的递增字段最大值存放的文件，如果该文件丢失，将重新抽取全部的数据。\n");
    printf("connstr1    已抽取数据的递增字段最大值存放的数据库的连接参数。connstr1和incfilename二选一，connstr1优先。");
    
    }
bool xmltoarg(const char *strxmlbuffer)
{
    memset(&starg,0,sizeof(struct st_arg));

    getxmlbuffer(strxmlbuffer,"connstr",starg.connstr,100);       // 数据源数据库的连接参数。
    if (strlen(starg.connstr)==0) { logfile.write("connstr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"charset",starg.charset,50);         // 数据库的字符集。
    if (strlen(starg.charset)==0) { logfile.write("charset is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"selectsql",starg.selectsql,1000);  // 从数据源数据库抽取数据的SQL语句。
    if (strlen(starg.selectsql)==0) { logfile.write("selectsql is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"fieldstr",starg.fieldstr,500);          // 结果集字段名列表。
    if (strlen(starg.fieldstr)==0) { logfile.write("fieldstr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"fieldlen",starg.fieldlen,500);         // 结果集字段长度列表。
    if (strlen(starg.fieldlen)==0) { logfile.write("fieldlen is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"bfilename",starg.bfilename,30);   // 输出xml文件存放的目录。
    if (strlen(starg.bfilename)==0) { logfile.write("bfilename is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"efilename",starg.efilename,30);    // 输出xml文件的前缀。
    if (strlen(starg.efilename)==0) { logfile.write("efilename is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"outpath",starg.outpath,255);        // 输出xml文件的后缀。
    if (strlen(starg.outpath)==0) { logfile.write("outpath is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"maxcount",starg.maxcount);       // 输出xml文件的最大记录数，可选参数。

    getxmlbuffer(strxmlbuffer,"starttime",starg.starttime,50);     // 程序运行的时间区间，可选参数。

    getxmlbuffer(strxmlbuffer,"incfield",starg.incfield,30);          // 递增字段名，可选参数。

    getxmlbuffer(strxmlbuffer,"incfilename",starg.incfilename,255);  // 已抽取数据的递增字段最大值存放的文件，可选参数。

    getxmlbuffer(strxmlbuffer,"connstr1",starg.connstr1,100);          // 已抽取数据的递增字段最大值存放的数据库的连接参数，可选参数。

    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);       // 进程心跳的超时时间。
    if (starg.timeout==0) { logfile.write("timeout is null.\n");  return false; }

    getxmlbuffer(strxmlbuffer,"pname",starg.pname,50);     // 进程名。
    if (strlen(starg.pname)==0) { logfile.write("pname is null.\n");  return false; }
    
        // 拆分starg.fieldstr到fieldname中。
    fieldname.splittocmd(starg.fieldstr,",");

    // 拆分starg.fieldlen到fieldlen中。
    fieldlen.splittocmd(starg.fieldlen,",");

    // 判断fieldname和fieldlen两个数组的大小是否相同。
    if (fieldlen.size()!=fieldname.size())
    {
        logfile.write("fieldstr和fieldlen的元素个数不一致。\n"); return false;
    }

    // 如果是增量抽取，incfilename和connstr1必二选一。
    if (strlen(starg.incfield)>0)
    {
        if ( (strlen(starg.incfilename)==0) && (strlen(starg.connstr1)==0) )
        {
            logfile.write("如果是增量抽取，incfilename和connstr1必二选一，不能都为空。\n"); return false;
        }
    }
    


    return true;

}
bool instarttime()
{
    // 程序运行的时间区间，例如02,13表示：如果程序启动时，踏中02时和13时则运行，其它时间不运行。
    if (strlen(starg.starttime)!=0)
    {
        string strhh24=ltime1("hh24");  // 获取当前时间的小时，如果当前时间是2023-01-08 12:35:40，将得到12。
        if (strstr(starg.starttime,strhh24.c_str())==0) return false;
    }       // 闲时：12-14时和00-06时。

    return true;

}


bool dminingoracle()
{
    // 1）准备抽取数据的SQL语句。
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare(starg.selectsql);
    // 2）绑定结果集的变量。
    vector<string> outvet(fieldname.size(),string());
    for(int ii=0;ii<fieldname.size();ii++)
    {
        stmt.bindout(ii,outvet[ii],stoi(fieldlen[ii]));
    }
    // 3）执行抽取数据的SQL语句。
    if(stmt.execute()!=0)
    {
        logfile.write("stmt.excute()fail\n%s\n%s",stmt.sql(),stmt.message());return false;
    }
    // 4）获取结果集中的记录，写入xml文件。先拼接文件名
    string strxmlfilename;
    int iseq=1;   //这个是待实现的分批写入文件
    cofile ofile;
    sformat(strxmlfilename,"%s/%s_%s_%s_%d.xml",\
                  starg.outpath,starg.bfilename,ltime1("yyyymmddhh24miss").c_str(),starg.efilename,iseq++);
    if(ofile.open(strxmlfilename)==false){logfile.write("open file failed\n");return false;}
    int ti=0;
    char strtime[30];

    while(true)
    {
        if(stmt.next()!=0)break;
        
        ofile.writeline("<data>\n");
        for(int ii=1;ii<fieldname.size();ii++)
        {
            ofile.writeline("<%s>%s</%s>",fieldname[ii-1].c_str(),outvet[ii].c_str(),fieldname[ii-1].c_str());
        }
        ofile.writeline("<height>110</height>");
        memset(strtime,0,sizeof(strtime));
        ltime(strtime,"yyyymmddhh24miss");
        ofile.writeline("<uptime>%s</uptime>",strtime);
        ofile.writeline("<endl/>\n");    // 写入每行的结束标志。
        ti++;
    }
    ofile.closeandrename();
    logfile.write("生成了%d条数据\n",ti);
    logfile.write("生成了文件%s\n",strxmlfilename.c_str());
    // 5）如果maxcount==0或者向xml文件中写入的记录数不足maxcount，关闭文件。
    //这个先不实现
    return true;
}








