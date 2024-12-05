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
    pactive.addpinfo(30);
    //获取当前时间和对应的过期时间
    string strtimeout=ltime1("yyyymmddhh24miss",0-(int)(atof(argv[3])*24*60*60));
    //打开对应的文件夹
    cdir dir;
    if(dir.opendir(argv[1],argv[2],10000,true)==false){printf("打开文件失败\n");};
    //遍历文件夹,删除对应的过期文件
    int ti=0;
    while (dir.readdir())
    {
        if(strtimeout>dir.m_mtime)
        {
            if(remove(dir.m_ffilename.c_str())==0){printf("删除文件成功\n"),ti++;}
            else{printf("删除文件失败\n");}
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
        printf("Using:/project/myidc/bin/清理程序 路径名 匹配规则 超时时间(天)\n\n");
        printf("要删除什么后缀的自己加就好了\n");
        printf("Example:/project/myidc/bin/清理程序 /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n");
        cout << R"(        /project/tools/bin/deletefiles /log/idc "*.log.20*" 0.02)" << endl;
        printf("        /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /log/idc \"*.log.20*\" 0.02\n");
        printf("        /project/tools/bin/procctl 300 /project/tools/bin/deletefiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n\n");

        printf("这是一个工具程序，用于删除历史的数据文件或日志文件。\n");
        printf("本程序把pathname目录及子目录中timeout天之前的匹配matchstr文件全部删除，timeout可以是小数。\n");
        printf("本程序不写日志文件，也不会在控制台输出任何信息。\n\n\n");
}