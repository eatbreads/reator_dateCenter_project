#include "_public.h"
using namespace idc;


clogfile logfile;               // 本程序运行的日志。
cpactive pactive;            // 进程的心跳



void EXIT(int sig);           // 程序退出和信号2、15的处理函数。
void help();
int main(int argc,char *argv[])
{
    //给出帮助文档
    if(argc<3){help();return -1;}

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

}