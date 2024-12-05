#include "_public.h"
using namespace idc;
using namespace std;
clogfile logfile;
int main(int argc, char *argv[]) 
{
    if(logfile.open("/tmp/log/demo1.log")==false)
    {
        cout<<"Error: Cannot open log file\n";
    }
    logfile.write("程序开始运行");
    for(int ii=0;ii<100;ii++)
    {
        sleep(1);
        logfile.write("这是第%d个数据\n",ii);
    }
    //cout<<"hello vscode\n";
    logfile.write("程序结束运行");
    return 0;
}