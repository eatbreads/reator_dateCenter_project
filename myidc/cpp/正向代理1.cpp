#include "_public.h"
using namespace idc;

// 代理路由参数的结构体。
struct st_route
{
    int    srcport;           // 源端口。
    char dstip[31];        // 目标主机的地址。
    int    dstport;          // 目标主机的端口。
    int    listensock;      // 源端口监听的socket。
}stroute;
vector<struct st_route> vroute;       // 代理路由的容器。
bool loadroute(const char *inifile);  // 把代理路由参数加载到vroute容器。

// 初始化服务端的监听端口。
int initserver(const int port);

int epollfd=0;      // epoll的句柄。

#define MAXSOCK  1024          // 最大连接数。
int clientsocks[MAXSOCK];       // 存放每个socket连接对端的socket的值。
int clientatime[MAXSOCK];       // 存放每个socket连接最后一次收发报文的时间。
string clientbuffer[MAXSOCK]; // 存放每个socket发送内容的buffer。

// 向目标地址和端口发起socket连接。
int conntodst(const char *ip,const int port);

void EXIT(int sig);     // 进程退出函数。

clogfile logfile;

//cpactive pactive;       // 进程心跳。
int main(int argc,char *argv[])
{
    if (argc != 3)
    {
        printf("\n");
        printf("Using :./inetd logfile inifile\n\n");
        printf("Sample:/project/myidc/cpp/正向代理 /tmp/inetd.log /etc/inetd.conf\n\n");
        printf("        /project/tools/bin/procctl 5 /project/tools/bin/inetd /tmp/inetd.log /etc/inetd.conf\n\n");
        printf("本程序的功能是正向代理，如果用到了1024以下的端口，则必须由root用户启动。\n");
        printf("logfile 本程序运行的日是志文件。\n");
        printf("inifile 路由参数配置文件。\n");

        return -1;
    }
    if(logfile.open(argv[1])==false){logfile.write("open logfile failed\n");return -1;}
    //解析配置文件进容器
    if(loadroute(argv[2])==false)
    {
        logfile.write("loadroute failed\n");
        return -1;
    }
    logfile.write("loadroute seccess\n");
    //创建epoll
    epollfd=epoll_create1(0);
    struct epoll_event ev;  
    //遍历容器,初始化所有监听句柄,然后监听读事件加入epoll
    for(auto &aa:vroute)
    {
        aa.listensock=initserver(aa.srcport);
        fcntl(aa.listensock,F_SETFL,fcntl(aa.listensock,F_GETFD,0)|O_NONBLOCK);
        if(aa.listensock<0)
        {
            logfile.write("initserver(%d) failed\n",aa.srcport);
            continue;
        }
        ev.data.fd=aa.listensock;
        ev.events=EPOLLIN;
        if(epoll_ctl(epollfd,EPOLL_CTL_ADD,aa.listensock,&ev)<0)
        {
            logfile.write("epoll_ctl(%d) failed\n",aa.listensock);
        
        }
        printf("上树成功fd=%d\n",aa.listensock);
    }
    struct epoll_event evs[10];
    while(true)
    {
        //如果是监听句柄
        int infds=epoll_wait(epollfd,evs,10,-1);
        printf("有事件发生\n");
        if(infds==-1)
        {
            logfile.write("epoll_wait failed\n");
            return -1;
        }
        
        for(int ii=0;ii<infds;ii++)
        {
            //查看是哪一个监听句柄,在route数组中匹配信息
            int jj=0;
            for(jj=0;jj<vroute.size();jj++)
            {
                
                if(vroute[jj].listensock==evs[ii].data.fd)
                {
                //初始化双端的socket
                    //从接收队列中取出客户端fd
                    printf("匹配到句柄%d\n",evs[ii].data.fd);
                    int clientfd=accept(evs[ii].data.fd,NULL,NULL);
                    if(clientfd<0)
                    {
                        logfile.write("accept(%d) failed\n",evs[ii].data.fd);
                        return -1;
                    }
                    ev.data.fd=clientfd;
                    ev.events=EPOLLIN;
                    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&ev)<0)
                    {
                        logfile.write("epoll_ctl(%d) failed\n",clientfd);
                        return -1;
                    }
                    printf("上树成功fd=%d\n",clientfd);
                    //向配置文件中对端的地址发起tcp连接
                    int dstfd=conntodst(vroute[jj].dstip,vroute[jj].dstport);
                    if(dstfd<0)
                    {
                        logfile.write("conntodst(%s,%d) failed\n",vroute[jj].dstip,vroute[jj].dstport);
                        return -1;
                    }
                    ev.data.fd=dstfd;
                    ev.events=EPOLLIN;
                    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,dstfd,&ev)<0)
                    {
                        logfile.write("epoll_ctl(%d) failed\n",dstfd);
                        return -1;
                    }
                //更新双端数组
                    clientsocks[clientfd]=dstfd;clientsocks[dstfd]=clientfd;
                    clientatime[clientfd]=time(NULL);clientatime[dstfd]=time(0);
                    
                    logfile.write("accept(%d) success\n",evs[ii].data.fd);
                
                }
            }
            //如果是其他句柄
            if(jj>=vroute.size())
            //如果是读事件
            {
                //读内容加入到缓冲区
                //注册读写事件,更新树mod
                ///跟新双端时间数组
            }
            //如果是写事件
            {
                //将缓冲区发送
                //删除缓冲区内部已经发送的部分
                //如果缓冲区没了
                //取消写事件,更新树mod
            }
        }

    }
}
// 初始化服务端的监听端口。
int initserver(const int port)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)
    {
        logfile.write("socket(%d) failed.\n",port); return -1;
    }

    int opt = 1; unsigned int len = sizeof(opt);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
    {
        logfile.write("bind(%d) failed.\n",port); close(sock); return -1;
    }

    if (listen(sock,5) != 0 )
    {
        logfile.write("listen(%d) failed.\n",port); close(sock); return -1;
    }

    return sock;
}

// 把代理路由参数加载到vroute容器。
bool loadroute(const char *inifile)
{
    cifile ifile;

    if (ifile.open(inifile)==false)
    {
        logfile.write("打开代理路由参数文件(%s)失败。\n",inifile); return false;
    }

    string strbuffer;
    ccmdstr cmdstr;

    while (true)
    {
        if (ifile.readline(strbuffer)==false) break;

        // 删除说明文字，#后面的部分。
        auto pos=strbuffer.find("#");
        if (pos!=string::npos) strbuffer.resize(pos);

        replacestr(strbuffer,"  "," ",true);    // 把两个空格替换成一个空格，注意第四个参数。
        deletelrchr(strbuffer,' ');                 // 删除两边的空格。

        // 拆分参数。
        cmdstr.splittocmd(strbuffer," ");
        if (cmdstr.size()!=3) continue;

        memset(&stroute,0,sizeof(struct st_route));
        cmdstr.getvalue(0,stroute.srcport);          // 源端口。
        cmdstr.getvalue(1,stroute.dstip);             // 目标地址。
        cmdstr.getvalue(2,stroute.dstport);         // 目标端口。

        vroute.push_back(stroute);
    }

    return true;
}

// 向目标地址和端口发起socket连接。
int conntodst(const char *ip,const int port)
{
    // 第1步：创建客户端的socket。
    int sockfd;
    if ( (sockfd = socket(AF_INET,SOCK_STREAM,0))==-1) return -1; 

    // 第2步：向服务器发起连接请求。
    struct hostent* h;
    if ( (h = gethostbyname(ip)) == 0 ) { close(sockfd); return -1; }
  
    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port); // 指定服务端的通讯端口。
    memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);

    // 把socket设置为非阻塞。
    fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK);

    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr))<0)
    {
        if (errno!=EINPROGRESS)
        {
            logfile.write("connect(%s,%d) failed.\n",ip,port); return -1;
        }
    }

    return sockfd;
}

void EXIT(int sig)
{
    logfile.write("程序退出，sig=%d。\n\n",sig);

    // 关闭全部监听的socket。
    for (auto &aa:vroute)
        if (aa.listensock>0) close(aa.listensock);

    // 关闭全部客户端的socket。
    for (auto aa:clientsocks)
        if (aa>0) close(aa);

    close(epollfd);   // 关闭epoll。

    exit(0);
}