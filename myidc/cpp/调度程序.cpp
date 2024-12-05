#include<cstdio>
#include<stdlib.h>
#include<cstring>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<sys/wait.h>
#include<iostream>
#include<string>

using namespace std;
int Fork()
{
    int ret=0;
    if((ret=fork())==-1)
    {
        cout<<"fork error"<<endl;
        exit(1);
    }
    //printf("ret=%d",ret);
    return ret;
}
int main(int argc, char *argv[])
{
    if(argc<3)
    {   //参数是当前程序名,调度时间间隔,要调度的程序,被调度的程序的参数
        printf("Using:./procctl timetvl program argv ...\n");
        printf("参数是当前程序名,调度时间间隔,要调度的程序,被调度的程序的参数\n");
        printf("Example:/project/myidc/bin/procctl 10 /usr/bin/tar zcvf /tmp/tmp.tgz /usr/include\n");
  	    printf("Example:/project/myidc/bin/procctl 60 /project/myidc/bin/crtsurfdata /project/idc/ini/stcode.ini /tmp/myidc/surfdata /log/idc/crtsurfdata.log xml\n");

        printf("本程序是服务程序的调度程序，周期性启动服务id程序或shell脚本。\n");

        printf("timetvl 运行周期，单位：秒。\n");
        printf("        被调度的程序运行结束后，在timetvl秒后会被procctl重新启动。\n");
        printf("        如果被调度的程序是周期性的任务，timetvl设置为运行周期。\n");
        printf("        如果被调度的程序是常驻内存的服务程序，timetvl设置小于5秒。\n");
        printf("program 被调度的程序名，必须使用全路径。\n");
        printf("...     被调度的程序的参数。\n");
        printf("注意，本程序不会被kill杀死，但可以用kill -9强行杀死。\n\n\n");
        return -1;
    }
    //printf("开始调度");

    //关闭信号和io,不希望被打扰和误杀
    for (int ii=0;ii<64;ii++)
    {
        close(ii);
        signal(ii,SIG_IGN); 
    }
    //生成子进程,父进程退出,使得把整个程序摆脱shell进程交给1号进程
    //相当于一开始的父进程只是为了做上面这件事,然后第一个子进程执行真正的调度程序
    if(Fork()!=0){printf("父进程退出\n");exit(0);}
    signal(SIGCHLD,SIG_DFL);
    //printf("开始调度\n");
    //获取程序参数
    char * temp[argc];
    for(int ii=2;ii<argc;ii++)
    {
        temp[ii-2]=argv[ii];//这里就是传入了程序名字和它的参数
    }
    temp[argc-2]=nullptr; //要保证最后有一个null指针
    //开始while循环+sleep开始调度程序
    while(true)
    {
        //printf("开始调度\n");
        if(Fork()==0)//子子进程
        {
            execv(argv[2],temp);
            exit(0);//这一行正常情况下不会执行
        }
        wait(nullptr);//这里传入nullptr即就是会等待这个子进程终止
        sleep(atoi(argv[1]));//这里sleep是为了让子进程睡眠一段时间
    }
    return 0;
}