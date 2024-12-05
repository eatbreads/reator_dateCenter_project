#include "_public.h"
using namespace idc;
using namespace std;


//打开日志
clogfile logfile;
//处理信号
void help();

int main(int argc, char *argv[]) 
{
    if(argc!=2){help();return -1;}
    closeioandsignal(true);
    //获取共享内存
    int shmid=0;
    if((shmid=shmget((key_t)SHMKEYP, MAXNUMP*sizeof(struct st_procinfo), 0666|IPC_CREAT))==-1)
    {
        logfile.write("获取共享内存失败\n");
    }
    struct st_procinfo*point = (st_procinfo*)shmat(shmid,0,0);

    //遍历共享内存,删除超时的共享内存
    int i;

    for(i=0;i<MAXNUMP;i++)
    {
        if(point[i].pid==0)continue;
        int iret=kill(point[i].pid,0);
        if (iret==-1)
        {
            logfile.write("进程pid=%d(%s)已经不存在。\n",point[i].pid,point[i].pname);
            memset(&point[i],0,sizeof(struct st_procinfo));   // 从共享内存中删除该记录。
            continue;
        } 
        if((time(0)-point[i].atime)>point[i].timeout)
        {
        // 一定要把进程的结构体备份出来，不能直接用共享内存中的值。
        struct st_procinfo tmp=point[i];
		if (tmp.pid==0) continue;  

        // 如果已超时。
        logfile.write("进程pid=%d(%s)已经超时。\n",tmp.pid,tmp.pname);

        // 发送信号15，尝试正常终止已超时的进程。
        kill(tmp.pid,15);     

        // 每隔1秒判断一次进程是否存在，累计5秒，一般来说，5秒的时间足够让进程退出。
        for (int jj=0;jj<5;jj++)
        {
            sleep(1);
            iret=kill(tmp.pid,0);     // 向进程发送信号0，判断它是否还存在。
            if (iret==-1) break;         // 进程已退出。
        }

        if (iret==-1)
            logfile.write("进程pid=%d(%s)已经正常终止。\n",tmp.pid,tmp.pname);
        else
        {   
            // 如果进程仍存在，就发送信号9，强制终止它。
            kill(tmp.pid,9);  
            logfile.write("进程pid=%d(%s)已经强制终止。\n",tmp.pid,tmp.pname);

            // 从共享内存中删除已超时进程的心跳记录。
            memset(point+i,0,sizeof(struct st_procinfo));
        }
        }
    }
    //把共享内存从当前进程中分离
    shmdt(point);

    return 0;  
}

void help()
{
        printf("\n");
        printf("Using:./checkproc logfilename\n");
        printf("\n这个程序由调度程序来启动,10秒调度一次,然后配合对应的日志文件\n");
        printf("Example:/project/myidc/bin/procctl 10 /project/myidc/bin/守护进程 /tmp/log/checkproc.log\n\n");

        printf("本程序用于检查后台服务程序是否超时，如果已超时，就终止它。\n");
        printf("注意：\n");
        printf("  1）本程序由procctl启动，运行周期建议为10秒。\n");
        printf("  2）为了避免被普通用户误杀，本程序应该用root用户启动。\n");
        printf("  3）如果要停止本程序，只能用killall -9 终止。\n\n\n");

}