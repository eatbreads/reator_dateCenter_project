#include "_public.h"
using namespace std;
using namespace idc;
void EXIT(int sig);     // 进程退出函数。

clogfile logfile;         // 本程序运行的日志。
// 初始化服务端的监听端口。
int initserver(const int port);
bool sendfile(int sock,const string &filename);
void EXIT(int sig);

struct st_client                   // 客户端的结构体。
{
    string clientip;                // 客户端的ip地址。
    int clientatime=0;     // 客户端最后一次活动的时间。
    string recvbuffer;           // 客户端的接收缓冲区。
    string sendbuffer;          // 客户端的发送缓冲区。
};

struct st_recvmesg              //这个是工作线程处理的时候使用临时message,之后要放进客户端的发送缓冲区
{
    int      sock=0;               // 客户端的socket。
    string message;            // 接收/发送的报文
    st_recvmesg()=default;
    st_recvmesg(int in_sock,string &in_message):sock(in_sock),message(in_message){ logfile.write("构造了报文。\n");}
};
void work(shared_ptr<st_recvmesg> ptr);

//线程类AA
class AA
{
private:
    //接收队列,对应的锁,和条件变量
    queue<shared_ptr<st_recvmesg>> m_rq;
    mutex m_rq_mutex;
    condition_variable m_rq_cv;
    //发送队列,对应的锁,用于线程通信的管道
    queue<shared_ptr<st_recvmesg>> s_rq;
    mutex s_rq_mutex;
    int s_workpipe[2]={0};              //这是内部线程间通信的管道
    //存放客户端的哈希表,俗称状态机
    unordered_map<int,st_client>clientmap;
    //原子变量,用来通知线程结束
    atomic_bool m_exit;
public:
    int s_readpipe[2]={0};              //这是主进程和内部线程通信的管道
    AA()
    {
        pipe(s_workpipe);
        pipe(s_readpipe);
        logfile.write("构造AA\n");
        m_exit=false;
    }
    void recvfunc(int listenport)   // 接收线程主函数，listenport-监听端口。把客户端的socket和请求报文放入接收队列，sock-客户端的socket，message-客户端的请求报文。
    {
    int listenfd=initserver(listenport);
    int epollfd=epoll_create1(0);
    struct epoll_event ev;
    ev.data.fd=listenfd;
    ev.events=EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&ev);
    ev.data.fd=s_readpipe[0];
    ev.events=EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,s_readpipe[0],&ev);
    
    struct epoll_event evs[10];
    while(true)
    {
        int infds=epoll_wait(epollfd,evs,10,-1);
        if(infds==-1)
        {
            logfile.write("epoll_wait failed\n");
            exit(-1);
        }
        //遍历事件
        //printf("infds:%d\n",infds);
        for(int ii=0;ii<infds;ii++)
        {
            //如果是读事件
            if(evs[ii].events & EPOLLIN)
            {
                //如果是监听句柄,就从接收缓冲区拿出clinet句柄注册读事件
                if(evs[ii].data.fd==listenfd)
                {
                    struct sockaddr_in client;
                    socklen_t len = sizeof(client);
                    int clientfd = accept(listenfd,(struct sockaddr*)&client,&len);
                    if(clientfd==-1)
                    {
                        logfile.write("accept 失败。\n");
                        
                    }
                    fcntl(clientfd,F_SETFL,fcntl(clientfd,F_GETFD,0)|O_NONBLOCK); 
                    string temp;
                    clientmap[clientfd].clientip=inet_ntoa(client.sin_addr); 
                    clientmap[clientfd].clientatime=time(0);
                    
                    ev.data.fd=clientfd;
                    ev.events=EPOLLIN;
                    epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&ev);
                    printf("epoll_ctl %d seccess。\n",clientfd);
                    continue;
                }
                //如果是外部exit函数发送过来的终止信号
                if(evs[ii].data.fd==s_readpipe[0])
                {
                    //向发送线程发送消息,使他退出工作
                    if(write(s_workpipe[1],(char*)"0",1)<=0)
                    {
                        logfile.write("write s_readpipe[0] failed\n");
                        exit(-1);
                    }
                    logfile.write("write s_readpipe[1] success\n");
                    cout<<"m_exit=true\n";
                    m_exit=true;
                    return ;
                }
                //如果是通信句柄,就读取消息并且加入读取数据放进接收缓冲区,并且注册epoll写事件

                //while(true)
                
                    char buffer[1024];
                    memset(&buffer,0,sizeof(buffer));
                    if(read(evs[ii].data.fd,buffer,sizeof(buffer))<=0)
                    {
                        printf("客户端连接断开。\n");
                        close(evs[ii].data.fd);
                        clientmap.erase(evs[ii].data.fd);
                        epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[ii].data.fd,0);
                        continue;
                    }
                    //printf("buffer:\n%s\n",buffer);
                    clientmap[evs[ii].data.fd].recvbuffer+=string(buffer);
                    //如果认为接收报文结束了,就执行插入接收队列操作
                    if( clientmap[evs[ii].data.fd].recvbuffer.compare( clientmap[evs[ii].data.fd].recvbuffer.length()-4,4,"\r\n\r\n")==0)                        //这里可以写判断结尾的条件,true就是无脑转发
                    {
                        cout<<"inrq start\n";
                        inrq(evs[ii].data.fd,clientmap[evs[ii].data.fd].recvbuffer);
                        //插入接收队列之后要把缓冲区重置                       
                        clientmap[evs[ii].data.fd].recvbuffer.clear();
                    }      
                    clientmap[evs[ii].data.fd].clientatime=time(0); 
            }

        }
    }
    }
    void inrq(int sock,string &message)     //放入接收队列
    {
        shared_ptr<st_recvmesg> ptr=make_shared<st_recvmesg>(sock,message);
        lock_guard<mutex> lock(m_rq_mutex);
        m_rq.push(ptr);
        m_rq_cv.notify_one();
    }
    void workfunc(int id)                      //工作函数,拿到接收队列内部内容
    {
        while(true)
        {
            shared_ptr<st_recvmesg> ptr;
            {
                unique_lock<mutex> lock(m_rq_mutex);
                while(m_rq.empty())
                {
                    m_rq_cv.wait(lock);     //就是这里wait出去的时候无论是在循环里面还是unique的作用域
                    if(m_exit==true)        //都是一直带着锁的,只有阻塞在wait是不带锁的
                    {                       //带锁出去while循环绕了一圈不满足条件又会回到wait(不带锁)
                        printf("work exit\n");
                        return;
                    }
                }

                ptr=m_rq.front();
                m_rq.pop();
            }
            //printf("start work for %s in thread%d\n",ptr->message.c_str(),id);
            ////////////////////////////////////////////////////////


            //在这里执行处理的代码,ptr里面存储了报文和对应发来的sock
            work(ptr);

            ////////////////////////////////////////////////////////
            insq(ptr->sock,ptr->message);
        }
    }
    void work(shared_ptr<st_recvmesg> ptr)
    {
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,\
        "HTTP/1.1 200 OK\r\n"
        "Server: server1\r\n"
        "Content-Length: %d\r\n"
        "Content-Type: text/html\r\n"
        "\r\n",filesize("/project/tools/cpp/demo/SURF_ZH_20230706154300_11350.xml"));
        write(ptr->sock,buffer,strlen(buffer));    // 把响应报文的状态行和头部信息发送给http客户端。
        write(ptr->sock,"hello this is my webserver of http\n",35);
        //cout<<"work() start\n";
        cifile ifile;
        string strbuffer;
        if(ifile.open("/project/tools/cpp/demo/SURF_ZH_20230706154300_11350.xml")==false)printf("open ifile failed\n");
        ptr->message.clear();
        // while(ifile.readline(strbuffer))
        // {
        //     ptr->message += strbuffer;
        //     strbuffer.clear();
        // }
        // //cout<<"work() end\n";
        sendfile(ptr->sock,"/project/tools/cpp/demo/SURF_ZH_20230706154300_11350.xml"); 

    }
    //void bizmain(connection &conn,shared_ptr<st_recvmesg> &ptr);
    void insq(int sock,string &message)        //把message压入发送队列
    {
        shared_ptr<st_recvmesg> ptr=make_shared<st_recvmesg>(sock,message);
        lock_guard<mutex>lock(s_rq_mutex);
        s_rq.push(ptr);
        ::write(s_workpipe[1],(char*)"0",1);
        //write(sock,message.c_str(),message.length());
        //printf("pushed:%s\n",ptr->message.c_str());
    }
    void sendfunc()                           //处理发送队列
    {
        int epollfd=epoll_create1(0);
        //把管道加入读事件
        struct epoll_event ev;               // 声明事件的数据结构。
        ev.events=EPOLLIN;                 // 读事件。
        ev.data.fd=s_workpipe[0];              // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
        epoll_ctl(epollfd,EPOLL_CTL_ADD,s_workpipe[0],&ev);  // 把监听的socket的事件加入epollfd中。
        struct epoll_event evs[10];
        while(true)
        {
        
            int infds=epoll_wait(epollfd,evs,10,-1);
            if(infds<0){logfile.write("sendfunc failed\n");return;}
        
            for(int ii=0;ii<infds;ii++)
            {
                //当管道有事件的时候把从发送队列里面拿东西然后注册写事件
                if(evs[ii].data.fd==s_workpipe[0])
                {
                    if(m_exit==true)
                    {
                        logfile.write("sendfunc exit\n");
                        return;
                    }
                    char cc;
                    read(s_workpipe[0], &cc, 1);
                    lock_guard<mutex> lock(s_rq_mutex);
                    string temp=s_rq.front()->message;
                    int socktemp=s_rq.front()->sock;
                    s_rq.pop();
                    clientmap[socktemp].sendbuffer+=temp;
                    //上树
                    ev.events=EPOLLOUT;
                    ev.data.fd=socktemp;
                    epoll_ctl(epollfd,EPOLL_CTL_ADD,socktemp,&ev);
                    continue;
                }

                //当有写事件发生的时候把缓冲区内容发出去
                if(evs[ii].events&EPOLLOUT)
                {
                    //这里一次发送多少可以自己调节
                    int writen=write(evs[ii].data.fd,clientmap[evs[ii].data.fd].sendbuffer.c_str(),100);
                    //删除对应的已发送的,并进行后续一些处理vc_str()
                    
                    //cout<<"send:"<<clientmap[evs[ii].data.fd].sendbuffer.substr(0,writen)<<endl;
                    clientmap[evs[ii].data.fd].sendbuffer.erase(0,writen);
                    if(clientmap[evs[ii].data.fd].sendbuffer.length()==0)
                    {
                        ev.data.fd=evs[ii].data.fd;
                        epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[ii].data.fd,&ev);
                        printf("delete %d\n",evs[ii].data.fd);
                    }
                    
                }
            }
        
        }
    }
    ~AA()
    {
        logfile.write("析构AA\n");
    }
};
st_recvmesg clientvet[200];
int listenfd;
AA aa;
int main(int argc,char *argv[])
{
    if (argc != 3)
    {
        printf("\n");
        printf("Using :./webserver logfile port\n\n");
        printf("Sample:./webserver /log/idc/webserver.log 5088\n\n");
        printf("        /project/tools/bin/procctl 5 /project/tools/bin/webserver /log/idc/webserver.log 5088\n\n");
        printf("基于HTTP协议的数据访问接口模块。\n");
        printf("logfile 本程序运行的日是志文件。\n");
        printf("port    服务端口，例如：80、8080。\n");

        return -1;
    }
    closeioandsignal();  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
    if (logfile.open(argv[1])==false)
    {
        printf("打开日志文件失败（%s）。\n",argv[1]); return -1;
    }
    
    
    thread t1(&AA::recvfunc,&aa,atoi(argv[2]));
    thread t2(&AA::workfunc,&aa,2);
    thread t3(&AA::workfunc,&aa,3);
    thread t4(&AA::workfunc,&aa,4);
    thread t5(&AA::sendfunc,&aa);

    while(true)//这里是为了保证主线程不比子线程先退出
    {
        sleep(30);
    }
    
    return 0;

}
int initserver(const int port)
{
        // 第1步：创建服务端的socket。 
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd==-1) 
    { 
        perror("socket"); return -1; 
    }

    int opt = 1; unsigned int len = sizeof(opt);

    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,len);
  
    // 第2步：把服务端用于通信的IP和端口绑定到socket上。 
    struct sockaddr_in servaddr;          // 用于存放服务端IP和端口的数据结构。
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;        // 指定协议。
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 服务端任意网卡的IP都可以用于通讯。
    servaddr.sin_port = htons(port);     // 指定通信端口，普通用户只能用1024以上的端口。
    // 绑定服务端的IP和端口。
    if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0 )
    { 
        perror("bind"); close(listenfd); return -1; 
    }
 
    // 第3步：把socket设置为可连接（监听）的状态。
    if (listen(listenfd,5) != 0 ) 
    { 
        perror("listen"); close(listenfd); return -1; 
    }
    return listenfd;
}

void EXIT(int sig)//在信号内部处理fd
{
    logfile.write("程序退出，sig=%d。\n\n",sig);
    //好像这些句柄是不需要关掉的,直接退出就好了

    // close(listenfd);
    // for(int ii=0;ii<30;ii++)
    // {
    //     if(clientvet[ii].sock==1)
    //     {
    //         close(ii);
    //         clientvet[ii].sock=0;
    //     }
    // }
    write(aa.s_readpipe[1],(char*)"o",1);   // 通知接收线程退出。
    sleep(1);    // 让线程们有足够的时间退出。
    exit(0);
}


bool sendfile(int sock,const string &filename)
{
    int  onread=0;         // 每次打算从文件中读取的字节数。 
    char buffer[1000];   // 存放读取数据的buffer，buffer的大小可参考硬盘一次读取数据量（4K为宜）。
    int  totalbytes=0;    // 从文件中已读取的字节总数。
    cifile ifile;                 // 读取文件的对象。

    int totalsize=filesize(filename);

    // 必须以二进制的方式操作文件。
    if (ifile.open(filename,ios::in|ios::binary)==false) return false;

    while (true)
    {
        memset(buffer,0,sizeof(buffer));

        // 计算本次应该读取的字节数，如果剩余的数据超过1000字节，就读1000字节。
        if (totalsize-totalbytes>1000) onread=1000;
        else onread=totalsize-totalbytes;

        // 从文件中读取数据。
        ifile.read(buffer,onread);   

        // 把读取到的数据发送给对端。
        send(sock,buffer,onread,0);

        // 计算文件已读取的字节总数，如果文件已读完，跳出循环。
        totalbytes=totalbytes+onread;

        if (totalbytes==totalsize) break;
    }

    return true;
}