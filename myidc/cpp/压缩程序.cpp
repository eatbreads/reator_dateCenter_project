#include "_public.h"
using namespace idc;


cpactive pactive;                 // 进程的心跳。

void EXIT(int sig);           // 程序退出和信号2、15的处理函数。
void help();
int main(int argc,char *argv[])
{
    //给出帮助文档
    if(argc!=4){help();return -1;}

    //处理信号,忽略全部信号但是打开2和15
    closeioandsignal(true);
    signal(SIGINT,EXIT);
    signal(SIGTERM,EXIT);
    pactive.addpinfo(200);
    //获取当前时间和对应的过期时间
    string strtimeout=ltime1("yyyymmddhh24miss",0-(int)(atof(argv[3])*24*60*60));
    //打开对应的文件夹
    cdir dir;
    if(dir.opendir(argv[1],argv[2],10000,true)==false){printf("打开文件失败\n");};
    //遍历文件夹,压缩对应的过期文件
    int ti=0;
    while (dir.readdir())
    {
        // 把文件的时间与历史文件的时间点比较，如果更早，并且不是压缩文件，就需要压缩。
        if ( (dir.m_mtime < strtimeout) && (matchstr(dir.m_filename,"*.gz")==false) )
        {
            // 压缩文件，调用操作系统的gzip命令。
            string strcmd="/usr/bin/gzip -f " + dir.m_ffilename + " 1>/dev/null 2>/dev/null";
            if (system(strcmd.c_str())==0)
                cout << "gzip " << dir.m_ffilename << "  ok.\n";
            else
                cout << "gzip " << dir.m_ffilename << " failed.\n"; 

            // 如果压缩的文件比较大，有几个G，需要时间可能比较长，所以，增加更新心跳的代码。
            pactive.uptatime();          
        }
    }
    printf("删除%d个文件\n",ti);
    return 0;
}


// 程序退出和信号2、15的处理函数。
void EXIT(int sig)
{
    printf("程序退出,sig=%d\n\n",sig);

    exit(0);
}

void help()
{
        printf("\n");
        printf("Using:/project/myidc/bin/压缩程序 路径名 匹配规则 超时时间(天)\n\n");
        printf("要压缩什么后缀的自己加就好了\n");
        printf("Example:/project/myidc/bin/压缩程序 /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n");
        cout << R"(        /project/tools/bin/deletefiles /log/idc "*.log.20*" 0.02)" << endl;
        printf("        /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /log/idc \"*.log.20*\" 0.02\n");
        printf("        /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n\n");

        printf("这是一个工具程序，用于压缩历史的数据文件或日志文件。\n");
        printf("本程序把pathname目录及子目录中timeout天之前的匹配matchstr文件全部压缩，timeout可以是小数。\n");
        printf("本程序不写日志文件，也不会在控制台输出任何信息。\n\n\n");
}