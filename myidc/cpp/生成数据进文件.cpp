#include "_public.h"
using namespace idc;
using namespace std;

//存放站点信息结构体
struct st_stcode//这样子命名可以使得名字为st_code,然后对象名字就是stcode
{
    /* data */
    char province[31];;//省份
    char obtid[11];//站点id
    char city[31];
    double lat;//经度
    double lon;//纬度
    double height;//高度
};
cpactive pactive;
//存放站点参数的容器
vector<st_stcode> st_vet;

//把站点里面的结构体加入容器的函数
bool loadstcode(const string& infile);

//观测结构体
struct st_surfdata
{
    char obtid[11];                // 站点代码。
    char ddatetime[15];            // 数据时间：格式yyyymmddhh24miss，精确到分钟，秒固定填00。
    int  t;                        // 气温：单位，0.1摄氏度。
    int  p;                        // 气压：0.1百帕。
    int  u;                        // 相对湿度，0-100之间的值。
    int  wd;                       // 风向，0-360之间的值。
    int  wf;                       // 风速：单位0.1m/s
    int  r;                        // 降雨量：0.1mm。
    int  vis;                      // 能见度：0.1米。
};
//存放观测结构体
vector<struct st_surfdata>  datalist;
//根据容器内部参数生成观测数据放进观测结构体
void crtsurfdata(); 
//观测时间
char strddatetime[15];
// 把容器datalist中的气象观测数据写入文件，outpath-数据文件存放的目录；
// datafmt-数据文件的格式，取值：csv、xml和json。
bool crtsurffile(const string& outpath,const string& datafmt);


//打开日志
clogfile logfile;
//处理信号
void EXIT(int sig); 
int time__=0;
void Debug()
{
    //sleep(1);
    time__++;
    cout << "Debug=" <<time__<< endl;
}
int main(int argc, char *argv[]) 
{   
    //参数为本文件,输入文件,输出文件,日志文件
    //帮助文档
    if(argc!=5)
    {
        cout<<"using:./crtsurfdata inifile outpath logfile\n";
        cout << "Examples:/project/myidc/bin/crtsurfdata /project/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml\n\n";

        cout << "inifile  气象站点参数文件名。\n";
        cout << "outpath  气象站点数据文件存放的目录。\n";
        cout << "logfile  本程序运行的日志文件名。\n";

        return -1;  
    }
  
    //处理信号,忽略全部信号但是打开2和15
    closeioandsignal(true);

    signal(SIGINT,EXIT);
  
    signal(SIGTERM,EXIT);
    //打开日志
    if(logfile.open(argv[3])==false)
    {
        cout<<"open log file error!\n";
        return -1;
    }
 
    logfile.write("程序开始运行\n");
    
    //读取参数
    // Debug();
    string inifile = argv[1];
    string outpath = argv[2];
    string logpath = argv[3];
    string format = argv[4];
    //调用loadstcode把输入文件写进st_vet中
    if(loadstcode(inifile)==false)
    {
        cout<<"load stcode error!\n";
        return -1;
    }

    logfile.write("已经载入容器内部\n");

    //生成观测时间
    memset(strddatetime,0,sizeof(strddatetime));
    ltime(strddatetime,"yyyymmddhh24miss");
    //cout<<strddatetime<<endl;
    strncpy(strddatetime+12,"00",2);
    //根据站点参数生成观测数据,同时放进容器里ps:和时间没关系,时间只是为了写日志
    crtsurfdata(); 
    //把容器内容写进文件
    if(crtsurffile(outpath,format)==false){logfile.write("写进文件失败\n");};
    logfile.write("写进文件成功\n");
    //关闭日志
    logfile.close();

    return 0;
}

void EXIT(int sig)
{
    logfile.write("收到信号退出,信号为:%d\n",sig);
    exit(0);
}
bool loadstcode(const string& infile)
{
    //打开文件
    cifile ifile;

    if(ifile.open(infile)==false)
    {
        cout<<"open file error!\n";
        logfile.write("打开%s失败",infile.c_str());
        return false;
    }
    string line;
    ifile.readline(line);
    ccmdstr cmdstr;
    st_stcode stcode;
    while(ifile.readline(line))
    {
        cmdstr.splittocmd(line,",");
        memset(&stcode,0,sizeof(stcode));
        cmdstr.getvalue(0,stcode.province,30);
        cmdstr.getvalue(1,stcode.obtid,10);
        cmdstr.getvalue(2,stcode.city,30);
        cmdstr.getvalue(3,stcode.lat);
        cmdstr.getvalue(4,stcode.lon);
        cmdstr.getvalue(5,stcode.height);
        st_vet.push_back(stcode);
    }

    return true;
}

//根据站点参数生成观测数据
void crtsurfdata()
{
    //随机数
    srand(time(0));              // 播随机数种子。

    st_surfdata stsurfdata;   // 观测数据的结构体。
    //生成数据
    int ti=0;
    for(auto &st:st_vet)
    {
        memcpy(stsurfdata.obtid,st.obtid,11);
        memcpy(stsurfdata.ddatetime,strddatetime,15);
        stsurfdata.t = rand()%100;
        stsurfdata.p = rand()%100;
        stsurfdata.u = rand()%100;
        stsurfdata.wd = rand()%360;
        stsurfdata.wf = rand()%100;
        stsurfdata.r = rand()%100;
        stsurfdata.vis = rand()%100;
        datalist.push_back(stsurfdata);
        ti++;
    }
    //写日志
    logfile.write("生成了%d条观测数据进容器\n",ti);
}
//写进文件
bool crtsurffile(const string& outpath,const string& datafmt)
{
    //拼接文件名,用进程号就不会重复了
    string strfilename=outpath+"/"+"SURF_ZH_"+strddatetime+"_"+to_string(getpid())+"."+datafmt;
    //打开文件
    cofile ofile;
    if(ofile.open(strfilename)==false){return false;}
    //这边目前只实现了xml格式,后续的json啥的后续再补充
    //拼接字符串成xml格式
    string xmlstr;
    int ti=0;
    for(auto &st:datalist)
    {
        xmlstr.clear();
        xmlstr.append("<data>");
        xmlstr.append("<obtid>"+string(st.obtid)+"</obtid>");
        xmlstr.append("<ddatetime>"+string(st.ddatetime)+"</ddatetime>");
        xmlstr.append("<t>"+to_string(st.t)+"</t>");
        xmlstr.append("<p>"+to_string(st.p)+"</p>");
        xmlstr.append("<u>"+to_string(st.u)+"</u>");
        xmlstr.append("<wd>"+to_string(st.wd)+"</wd>");
        xmlstr.append("<wf>"+to_string(st.wf)+"</wf>");
        xmlstr.append("<r>"+to_string(st.r)+"</r>");
        xmlstr.append("<vis>"+to_string(st.vis)+"</vis>");
        xmlstr.append("</data>\n");
        // ofile.writeline("<obtid>%s</obtid><ddatetime>%s</ddatetime><t>%.1f</t><p>%.1f</p><u>%d</u>"\
        //                            "<wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl/>\n",\
        //                             aa.obtid,aa.ddatetime,aa.t/10.0,aa.p/10.0,aa.u,aa.wd,aa.wf/10.0,aa.r/10.0,aa.vis/10.0);

        ti++;
        //cout<<xmlstr<<endl;
        //ofile.writeline("这是第%d条",ti);
        if(ofile.writeline(xmlstr.c_str())==false)
        {return false;}
    }
    logfile.write("往%s里面写进了%d条记录\n",strfilename.c_str(),ti);
    ofile.closeandrename();
    return true;
}