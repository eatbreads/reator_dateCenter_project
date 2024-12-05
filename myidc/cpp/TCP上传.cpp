#include "_public.h"
using namespace idc;

//遇到了世纪大难题,这里是往我自己之前的reactor发送数据,但是这个的tcpclient不适配我的server,他需要在前面加上报文长度
//解决办法就可以去修改一下原来的server,让他返回数据的时候也加一下报文长度
clogfile logfile;               // 本程序运行的日志。
cpactive pactive;            // 进程的心跳
ctcpclient tcpclient; //客户端
//存放xml参数的结构体,在这里不用string,char可以设置的大一点
// 程序运行的参数结构体。
struct st_arg
{
    int    clienttype;                // 客户端类型，1-上传文件；2-下载文件，本程序固定填1。
    char ip[31];                       // 服务端的IP地址。
    int    port;                        // 服务端的端口。
    char clientpath[256];       // 本地文件存放的根目录。 /data /data/aaa /data/bbb
    int    ptype;                      // 文件上传成功后本地文件的处理方式：1-删除文件；2-移动到备份目录。
    char clientpathbak[256]; // 文件成功上传后，本地文件备份的根目录，当ptype==2时有效。
    bool andchild;                 // 是否上传clientpath目录下各级子目录的文件，true-是；false-否。
    char matchname[256];    // 待上传文件名的匹配规则，如"*.TXT,*.XML"。
    char srvpath[256];           // 服务端文件存放的根目录。/data1 /data1/aaa /data1/bbb
    int    timetvl;                    // 扫描本地目录文件的时间间隔（执行文件上传任务的时间间隔），单位：秒。 
    int    timeout;                  // 进程心跳的超时时间。
    char pname[51];             // 进程名，建议用"tcpputfiles_后缀"的方式。
} starg;
string sendbuffer;
string readbuffer;
// 把xml解析到参数starg结构中。
bool _xmltoarg(const char *strxmlbuffer);

// 向服务端发送登录报文，把客户端程序的参数传递给服务端。
bool login(const char *);
void EXIT(int sig);           // 程序退出和信号2、15的处理函数。
void help();
int main(int argc,char *argv[])
{
    //给出帮助文档
    if(argc<3){help();return -1;}
    //处理信号,忽略全部信号但是打开2和15
    //closeioandsignal();
    signal(SIGINT,EXIT);
    signal(SIGTERM,EXIT);
    //打开日志
    if(logfile.open(argv[1])==false)
    {
        cout<<"open log file error!\n";
        return -1;
    }
    //解析xml获得参数
    if(_xmltoarg(argv[2])==false){return -1;}
    //申请tcp连接,完成三次握手
    if(tcpclient.connect(starg.ip,starg.port)==false){logfile.write("connect fail\n");return -1;}
    logfile.write("connect seccess\n");
    //tcp发送登录命令
    if(login("登录")==false){logfile.write("login fail\n");return -1;}
    logfile.write("login seccess\n");
    //发送想要上传的文件路径,和各种信息

    //while循环开始上传

    //收尾工作

    logfile.write("程序开始运行\n");
    return 0;
}


// 程序退出和信号2、15的处理函数。
void EXIT(int sig)
{
    logfile.write("程序退出,sig=%d\n\n",sig);

    exit(0);
}

void help()
{
    printf("\n");
    printf("Using:/project/myidc/bin/TCP上传 logfilename xmlbuffer\n\n");

    printf("Sample:/project/myidc/bin/procctl 20 /project/myidc/bin/TCP上传 /log/idc/_surfdata.log "\
              "\"<ip>192.168.171.100</ip><port>5005</port>"\
              "<clientpath>/tmp/client</clientpath><ptype>1</ptype>"
              "<srvpath>/tmp/server</srvpath>"\
              "<andchild>true</andchild><matchname>*.xml,*.txt,*.csv</matchname><timetvl>10</timetvl>"\
              "<timeout>50</timeout><pname>TCP上传_surfdata</pname>\"\n\n");

    printf("本程序是数据中心的公共功能模块，采用tcp协议把文件上传给服务端。\n");
    printf("logfilename   本程序运行的日志文件。\n");
    printf("xmlbuffer     本程序运行的参数，如下：\n");
    printf("ip            服务端的IP地址。\n");
    printf("port          服务端的端口。\n");
    printf("ptype         文件上传成功后的处理方式：1-删除文件；2-移动到备份目录。\n");
    printf("clientpath    本地文件存放的根目录。\n");
    printf("clientpathbak 文件成功上传后，本地文件备份的根目录，当ptype==2时有效。\n");
    printf("andchild      是否上传clientpath目录下各级子目录的文件，true-是；false-否，缺省为false。\n");
    printf("matchname     待上传文件名的匹配规则，如\"*.TXT,*.XML\"\n");
    printf("srvpath       服务端文件存放的根目录。\n");
    printf("timetvl       扫描本地目录文件的时间间隔，单位：秒，取值在1-30之间。\n");
    printf("timeout       本程序的超时时间，单位：秒，视文件大小和网络带宽而定，建议设置50以上。\n");
    printf("pname         进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n");

}

bool _xmltoarg(const char *strxmlbuffer)
{
    memset(&starg,0,sizeof(starg));
    // 解析xml
    getxmlbuffer(strxmlbuffer,"ip",starg.ip);
    if(strlen(starg.ip)==0){logfile.write("ip错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"port",starg.port);
    if(starg.port==0){logfile.write("port错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"clientpath",starg.clientpath);
    if(strlen(starg.clientpath)==0){logfile.write("clientpath错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"ptype",starg.ptype);
    if(starg.ptype==0){logfile.write("ptype错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"srvpath",starg.srvpath);
    if(strlen(starg.srvpath)==0){logfile.write("srvpath错误\n");return false;}
    //getxmlbuffer(strxmlbuffer,"clientpathbak",starg.clientpathbak);
    //if(strlen(starg.clientpathbak)==0){logfile.write("clientpathbak错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"andchild",starg.andchild);
    if(starg.andchild==0){logfile.write("andchild错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"matchname",starg.matchname);
    if(strlen(starg.matchname)==0){logfile.write("matchname错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"timetvl",starg.timetvl);
    if(starg.timetvl==0){logfile.write("timetvl错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);
    if(starg.timeout==0){logfile.write("timeout错误\n");return false;}
    getxmlbuffer(strxmlbuffer,"pname",starg.pname);
    if(strlen(starg.pname)==0){logfile.write("pname错误\n");return false;}
    
    
    


    return true;
}

// 向服务端发送登录报文，把客户端程序的参数传递给服务端。
bool login(const char *strxmlbuffer)
{
    string xmlbuffer=strxmlbuffer;
    // 发送xml
    if(tcpclient.write(xmlbuffer.c_str(),xmlbuffer.length())==false){logfile.write("send xml error\n");return false;}
    // 接收xml
    if(tcpclient.read(readbuffer)==false){logfile.write("recv xml error\n");return false;}
    logfile.write(readbuffer.c_str());
    logfile.write("成功\n");
    // 解析xml
    //if(readbuffer.find("login")==string::npos){logfile.write("login error\n");return false;}
    
    return true;
}