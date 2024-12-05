#include "_public.h"
#include "_ooci.h"
#include "_tools.h"
using namespace idc;

//读取列的对象
ctcols tcols;
clogfile logfile;               // 本程序运行的日志。
cpactive pactive;            // 进程的心跳
connection conn;            // 数据源数据库。。
struct st_arg
{
    char connstr[101];          // 数据库的连接参数。
    char charset[51];            // 数据库的字符集。
    char inifilename[301];    // 数据入库的参数配置文件。
    char xmlpath[301];         // 待入库xml文件存放的目录。
    char xmlpathbak[301];   // xml文件入库后的备份目录。
    char xmlpatherr[301];    // 入库失败的xml文件存放的目录。
    int  timetvl;                    // 本程序运行的时间间隔，本程序常驻内存。
    int  timeout;                   // 本程序运行时的超时时间。
    char pname[51];            // 本程序运行时的程序名。
} starg;
// 数据入库参数的结构体。
struct st_xmltotable
{
    char filename[101];    // xml文件的匹配规则，用逗号分隔。
    char tname[31];         // 待入库的表名。
    int    uptbz;                // 更新标志：1-更新；2-不更新。
    char execsql[301];     // 处理xml文件之前，执行的SQL语句。
} stxmltotable;
bool findxmltotable(const string &xmlfilename);
vector<struct st_xmltotable> vxmltotable;             // 数据入库的参数的容器。
bool xmltoarg(const char *strxmlbuffer);            // 把xml解析到参数starg结构中。
bool loadxmltotable();                                             // 把数据入库的参数配置文件starg.inifilename加载到vxmltotable容器中。


string strinsertsql;                        // 插入表的SQL语句。
string strupdatesql;                      // 更新表的SQL语句。
void crtsql();                                 // 拼接插入和更新表数据的SQL。
void help();                    //帮助文档
bool xmltodb();                 //程序运行主函数
// 处理xml文件的子函数，返回值：0-成功，其它的都是失败，失败的情况有很多种，暂时不确定。
bool _xmltodb(const string &fullfilename,const string &filename);
// <obtid>58015</obtid><ddatetime>20230508113000</ddatetime><t>141</t><p>10153</p><u>44</u><wd>67</wd><wf>106</wf><r>9</r><vis>102076</vis><keyid>6127135</keyid>
vector<string> vcolvalue;           // 存放从xml每一行中解析出来的字段的值，将用于插入和更新表的SQL语句绑定变量。
sqlstatement stmtins,stmtupt;    // 插入和更新表的sqlstatement语句。
void preparesql();          //准备插入和更新的sql语句(绑定变量)
bool execsql();                 //在处理xml文件之前,如果stxmltotable的execsql不为空,就执行他
// 解析xml，存放在已绑定的输入变量vcolvalue数组中。
void splitbuffer(const string &strBuffer);
void EXIT(int sig);           // 程序退出和信号2、15的处理函数。
int main(int argc,char *argv[])
{
    // 帮助文档。
    if (argc!=3){help();return -1;}
    //查询结果上传本地
    //处理信号,忽略全部信号但是打开2和15
    //closeioandsignal(true);
    signal(SIGINT,EXIT);
    signal(SIGTERM,EXIT);
    //打开日志
    if(logfile.open(argv[1])==false)
    {
        cout<<"open log file error!\n";
        return -1;
    }
    //解析xml获得参数
    //logfile.write("开始解析参数\n");
    if(xmltoarg(argv[2])==false){logfile.write("解析参数失败\n");return -1;}
    // 判断当前时间是否在程序运行的时间区间内。
    // 连接数据源的数据库。
    if (conn.connecttodb(starg.connstr,starg.charset)!=0)
    {
        logfile.write("connect database(%s) failed.\n%s\n",starg.connstr,conn.message()); EXIT(-1);
    }
    //logfile.write("connect database(%s) ok.\n",starg.connstr);
    //打开文件获取参数文件的内容
    //拼接sql语句实现入库
    logfile.write("开始执行主函数\n");
    if(xmltodb()==false){logfile.write("xmltodb failed\n");return -1;}
    logfile.write("程序正常退出\n");
    //logfile.write("execute success 插入了%d条数据,耗费时间为%.2f\n",ti,timer.elapsed());
    return 0;
}




void EXIT(int sig)              // 程序退出和信号2、15的处理函数。i
{
    logfile.write("程序退出,sig=%d\n\n",sig);

    exit(0);
}
void help()
{
    printf("Using:/project/myidc/db/xml入库 logfilename xmlbuffer\n\n");

    printf("Sample:/project/myidc/bin/procctl 10 /project/myidc/db/xml入库 /log/idc/xmltodb_vip.log "\
              "\"<connstr>idc/idcpwd</connstr><charset>Simplified Chinese_China.AL32UTF8</charset>"\
              "<inifilename>/project/idc/ini/xmltodb.xml</inifilename>"\
              "<xmlpath>/idcdata/xmltodb/vip</xmlpath><xmlpathbak>/idcdata/xmltodb/vipbak</xmlpathbak>"\
              "<xmlpatherr>/idcdata/xmltodb/viperr</xmlpatherr>"\
              "<timetvl>5</timetvl><timeout>50</timeout><pname>xmltodb_vip</pname>\"\n\n");

    printf("本程序是共享平台的公共功能模块，用于把xml文件入库到Oracle的表中。\n");
    printf("logfilename   本程序运行的日志文件。\n");
    printf("xmlbuffer     本程序运行的参数，用xml表示，具体如下：\n\n");

    printf("connstr     数据库的连接参数，格式：username/passwd@tnsname。\n");
    printf("charset     数据库的字符集，这个参数要与数据源数据库保持一致，否则会出现中文乱码的情况。\n");
    printf("inifilename 数据入库的参数配置文件。\n");
    printf("xmlpath     待入库xml文件存放的目录。\n");
    printf("xmlpathbak  xml文件入库后的备份目录。\n");
    printf("xmlpatherr  入库失败的xml文件存放的目录。\n");
    printf("timetvl     扫描xmlpath目录的时间间隔（执行入库任务的时间间隔），单位：秒，视业务需求而定，2-30之间。\n");
    printf("timeout     本程序的超时时间，单位：秒，视xml文件大小而定，建议设置30以上。\n");
    printf("pname       进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n");
    }
bool xmltoarg(const char *strxmlbuffer)        // 把xml解析到参数starg结构中
{
    memset(&starg,0,sizeof(struct st_arg));

    getxmlbuffer(strxmlbuffer,"connstr",starg.connstr,100);
    if (strlen(starg.connstr)==0) { logfile.write("connstr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"charset",starg.charset,50);
    if (strlen(starg.charset)==0) { logfile.write("charset is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"inifilename",starg.inifilename,300);
    if (strlen(starg.inifilename)==0) { logfile.write("inifilename is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"xmlpath",starg.xmlpath,300);
    if (strlen(starg.xmlpath)==0) { logfile.write("xmlpath is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"xmlpathbak",starg.xmlpathbak,300);
    if (strlen(starg.xmlpathbak)==0) { logfile.write("xmlpathbak is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"xmlpatherr",starg.xmlpatherr,300);
    if (strlen(starg.xmlpatherr)==0) { logfile.write("xmlpatherr is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"timetvl",starg.timetvl);
    if (starg.timetvl< 2) starg.timetvl=2;   
    if (starg.timetvl>30) starg.timetvl=30;

    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);
    if (starg.timeout==0) { logfile.write("timeout is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"pname",starg.pname,50);
    if (strlen(starg.pname)==0) { logfile.write("pname is null.\n"); return false; }

    return true;
}
//xml入库的工作函数,里卖弄包含了打开目录,遍历文件,对每个文件查找配置参数和对每个文件执行入库操作
bool xmltodb()
{
    //打开配置参数文件
    if(loadxmltotable()==false){logfile.write("解析配置文件失败\n");return false;}
    logfile.write("解析配置文件成功\n");
    //打开目录,循环遍历每一个文件进行入库
    //执行          //执行失败的文件放进err目录                      
    //把文件放入备份目录(成功的)
    cdir dir;
    if(dir.opendir(starg.xmlpath,"*.xml",10000,false,true)==false){logfile.write("open directory failed\n");return false;}
    while(true)
    {
        if(dir.readdir()==false){logfile.write("目录中没有了\n");break;}
        //处理文件函数dir.m_ffilename,根据函数名从容器中获取表名等入库参数
        if(findxmltotable(dir.m_filename)==false){logfile.write("findxmltotable fail\n");}
        logfile.write("查找到了入库参数\n");
        //文件入库子函数
        if(_xmltodb(dir.m_ffilename,dir.m_filename)==false){logfile.write("xml入库失败,文件名为%s\n",dir.m_ffilename.c_str());return false;}

        logfile.write("写入了一条文件:%s\n",dir.m_ffilename.c_str());
    }
    return true;
    
}
bool loadxmltotable()
{
    vxmltotable.clear();
    //打开配置参数文件
    cifile ifile;
    string buffer;      //临时存放xml语句
    if(ifile.open(starg.inifilename)==false){logfile.write("打开配置文件失败\n");return false;}
    while(true)
    {
        if(ifile.readline(buffer,"<endl/>")==false)break;
        memset(&stxmltotable,0,sizeof(st_xmltotable));
        getxmlbuffer(buffer,"filename",stxmltotable.filename,100);
        getxmlbuffer(buffer,"tname",stxmltotable.tname,30);
        getxmlbuffer(buffer,"uptbz",stxmltotable.uptbz);
        getxmlbuffer(buffer,"execsql",stxmltotable.execsql,300);
        vxmltotable.push_back(stxmltotable);
    }
    logfile.write("read%s ok",starg.inifilename);
    return true;
}
//给定文件名,在配置参数结构体里面找到对应文件名的文件的配置数据(对应的数据库啥的)
bool findxmltotable(const string &xmlfilename)
{
    for ( auto &aa : vxmltotable)
    {
        if (matchstr(xmlfilename,aa.filename)==true)
        {
            stxmltotable=aa;  
            return true;
        }
    }
    logfile.write("没有在数组找到对应的配置文件\n");
    return false;
}

//单个入库子函数
bool _xmltodb(const string &fullfilename,const string &filename)
{
        //根据读取到的配置参数读取数据库字典
        if(tcols.allcols(conn,stxmltotable.tname)==false){logfile.write("读取失败");return false;}
        if(tcols.pkcols(conn,stxmltotable.tname)==false){logfile.write("读取主键失败");return false;}

        //拼接sql语句
        if(tcols.m_allcols.size()==0){logfile.write("待入库的表不存在\n");return false;}
        //拼接插入和更新表的sql语句,同时绑定好变量
        crtsql();
        preparesql();
        //处理文件之前,判断是否有现成的已经指定好的sql语句,这个是预先制定好的,可能是要先于其他的sql执行的
        //if(execsql()==false){return false;}
        //打开文件
        cifile ifile ;
        //如果打开文件失败,则需要回滚事务,因为之前配置文件里面的sql可能被执行了
        if(ifile.open(fullfilename)==false){logfile.write("打开文件失败\n");conn.rollback();return false;}
        string strbuffer;   //存放从xml文件里面读取的一行
        int i=0;
        int b=0;
        while(true)
        {
            //读取数据
            if(ifile.readline(strbuffer,"<endl/>")==false)break;
            //解析strbuffer里面的xml
            splitbuffer(strbuffer);
            
            //执行插入或者更新的sql语句
            if(stmtins.execute()!=0)
            {
                if(stmtins.rc()==1)//违反了唯一性约束
                {
                    if(stxmltotable.uptbz==1)//如果配置文件里面要求更新表
                    {
                        if(stmtupt.execute()!=0)//更新失败一般是数据啥的错了
                        {   
                            logfile.write("更新失败,%s\n",stmtupt.message());
                        }
                        else b++;
                    }
                }
                else//insert失败的其他原因
                {
                    i++;
                    //logfile.write("insert失败,%s\n",stmtins.message());
                }
            }
        }
        logfile.write("失败了%d次,更新了%d次",i,b);
    //提交事务
    conn.commit();
    
    return true;
}
void crtsql()                                // 拼接插入和更新表数据的SQL。
{
    // 拼接插入表的SQL语句。 
    // insert into T_ZHOBTMIND1(obtid,ddatetime,t,p,u,wd,wf,r,vis,keyid) 
    //                                   values(:1,to_date(:2,'yyyymmddhh24miss'),:3,:4,:5,:6,:7,:8,:9,SEQ_ZHOBTMIND1.nextval)
    string strinsertp1;    // insert语句into后面的字段列表。
    string strinsertp2;    // insert语句的value后面的字段值列表。
    int colseq =1;
    //遍历所有容器,拿到into字段名和value字段名
    for(auto &aa:tcols.m_vallcols)
    {
        //upttime字段的缺省值时sysdate就不需要处理,因为他有默认值不需要我们自己插入
        if(strcmp(aa.colname,"upttime")==0)continue;
        strinsertp1=strinsertp1+aa.colname+",";
        //要单独区分keyid和日期date,他们的插入不太一样,在这个for循环里面需要特殊处理
        if(strcmp(aa.colname,"keyid")==0)
        {
            //说实话这个怎么设计的没太看懂,为什么和本表同名的序列生成器要这样子表名+2的操作,此时的stxmltotable储存的是当前
            //正在执行的文件的入库参数
            strinsertp2 = strinsertp2 + sformat("SEQ_%s.nextval",stxmltotable.tname+2) + ",";
        }
        else 
        {
            if(strcmp(aa.datatype,"date")==0)
                strinsertp2 = strinsertp2+sformat("to_date(:%d,'yyyymmddhh24miss')",colseq) + ",";  
            else strinsertp2 = strinsertp2 + sformat(":%d",colseq) + ",";
            colseq++;
        }
    }
    //把strinsertp1尾部逗号删除
    deleterchr(strinsertp1,','); deleterchr(strinsertp2,',');
    //拼接insertSQL语句。
    sformat(strinsertsql,"insert into %s(%s) values(%s)\n",stxmltotable.tname,strinsertp1.c_str(),strinsertp2.c_str());
    
    // update T_ZHOBTMIND1 set t=:1,p=:2,u=:3,wd=:4,wf=:5,r=:6,vis=:7 
    //                             where obtid=:8 and ddatetime=to_date(:9,'yyyymmddhh24miss')
    if(stxmltotable.uptbz!=1)return ;
    //拼接updataSQL语句
    sformat(strupdatesql,"update %s set ",stxmltotable.tname);
    //重置绑定序号
    colseq=1;
    for(auto &aa:tcols.m_vallcols)
    {
        // 如果是主键字段，不需要拼接在set的后面。
        if (aa.pkseq!=0) continue;

        // 如果字段名是keyid，不需要更新，不处理。
        if (strcmp(aa.colname,"keyid")==0) continue;

        // 如果字段名是upttime，字段直接赋值sysdate。
        if (strcmp(aa.colname,"upttime")==0)
        {
            strupdatesql = strupdatesql +"upttime=sysdate,";  continue;
        }

        // 其它字段拼接在set后面，需要区分date字段和非date字段。
        if (strcmp(aa.datatype,"date")!=0)    // 非date字段。
            strupdatesql=strupdatesql+sformat("%s=:%d,",aa.colname,colseq);
        else    // date字段。
            strupdatesql=strupdatesql+sformat("%s=to_date(:%d,'yyyymmddhh24miss'),",aa.colname,colseq);

        colseq++;   // 绑定变量的序号加1。
    }
    deleterchr(strupdatesql,',');
        // c）拼接update语句where后面的部分。
    strupdatesql = strupdatesql +  " where 1=1 ";      // 用1=1是为了后面的拼接方便，这是常用的处理方法。
    // where obtid=:8 and ddatetime=to_date(:9,'yyyymmddhh24miss')
    // where 1=1 and obtid=:8 and ddatetime=to_date(:9,'yyyymmddhh24miss')

    for (auto &aa : tcols.m_vallcols)  // 遍历表全部字段的容器。
    {
        if (aa.pkseq==0) continue;   // 如果不是主键字段，跳过。

        // 把主键字段拼接到update语句中，需要区分date字段和非date字段。
        if (strcmp(aa.datatype,"date")!=0)
             strupdatesql = strupdatesql + sformat(" and %s=:%d",aa.colname,colseq);
        else
             strupdatesql = strupdatesql + sformat(" and %s=to_date(:%d,'yyyymmddhh24miss')",aa.colname,colseq);

        colseq++;    // 绑定变量的序号加1。
    }
    logfile.write("insertsql语句为%s\n",strinsertsql.c_str());
    
    logfile.write("updatesql语句为%s\n",strupdatesql.c_str());
    return;
}


void preparesql()         //准备插入和更新的sql语句(绑定变量)
{
    //给vector扩容
    vcolvalue.resize(tcols.m_allcols.size());
    //准备插入表的sql语句,输入变量
    stmtins.connect(&conn);
    stmtins.prepare(strinsertsql);
    int colseq=1;
    //logfile.write("\n%s\n",stmtins.sql());
    //遍历m_allcols,把东西注入vcolvalue
    for(int ii=0;ii<tcols.m_vallcols.size();ii++)
    {
        //upttime和keyid这俩字段不需要绑定参数
        if((strcmp(tcols.m_vallcols[ii].colname,"upttime")==0)||(strcmp(tcols.m_vallcols[ii].colname,"keyid")==0))
        {continue;}
        stmtins.bindin(colseq,vcolvalue[ii],tcols.m_vallcols[ii].collen);//绑定输入变量
        
        colseq++;
    }
    logfile.write("绑定了插入表的参数\n");
    //准备插入表的sql语句
        //如果指定了不需要更新则直接返回即可
    if(stxmltotable.uptbz!=1)return;
    stmtupt.connect(&conn);
    stmtupt.prepare(strupdatesql);
    //绑定set输入参数
    colseq=1;
    for(int ii=0;ii<tcols.m_vallcols.size()-2;ii++)//遍历所有字段
    {
        //如果是主键则不需要拼接在set后面
        //if(tcols.m_vallcols[ii].pkseq!=0)continue;
        //如果是upttime和keyid则不需要处理
        //if((strcmp(tcols.m_vallcols[ii].colname,"upttime")==0)||(strcmp(tcols.m_vallcols[ii].colname,"keyid")==0))
        //if(strcmp(tcols.m_vallcols[ii].colname,"keyid")==0)
        //{continue;}
        stmtupt.bindin(colseq,vcolvalue[ii+1],tcols.m_vallcols[ii+1].collen);
        
        colseq++;
    }
    
    logfile.write("绑定了set的参数\n");
    //绑定where的输入参数
    // for(int ii=0;ii<tcols.m_vallcols.size();ii++)
    // {
    //     //如果不是主键字段则跳过,只有主键需要凭借在where后面,和上面的set正好相反
    //     if(tcols.m_vallcols[ii].pkseq==0)continue;
    //     stmtupt.bindin(colseq,vcolvalue[ii],tcols.m_vallcols[ii].collen);
        
    //     colseq++;
    // }
    stmtupt.bindin(colseq,vcolvalue[0],tcols.m_vallcols[0].collen);
    logfile.write("绑定了where参数\n");
    return ;
}

bool execsql()                 //在处理xml文件之前,如果stxmltotable的execsql不为空,就执行他
{
    if(strlen(stxmltotable.execsql)==0)return true;
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare(stxmltotable.execsql);
    if(stmt.execute()!=0)
    {
        logfile.write("执行配置文件的sql失败,失败原因为%s\n",stmt.message());
        return false;
    }
    return true;
}

// 解析xml，存放在已绑定的输入变量vcolvalue数组中。
void splitbuffer(const string &strBuffer)
{
    //char chartemp[300];
    string temp;
    for(int ii=0;ii<tcols.m_vallcols.size();ii++)
    {
       // memset(chartemp,0,300);
        getxmlbuffer(strBuffer,tcols.m_vallcols[ii].colname,temp,tcols.m_vallcols[ii].collen);
        
        //如果是日期date就只要提取数字,因为里面可能是yyyymmddhh24miss
        if(strcmp(tcols.m_vallcols[ii].datatype, "date")==0)
        {
            picknumber(temp,temp,false,false);
        }
        else if(strcmp(tcols.m_vallcols[ii].datatype,"date")==0)
        {
            picknumber(temp,temp,true,true);
        }
        vcolvalue[ii]=temp.c_str();
        //vcolvalue[ii]=chartemp;
    }
    return ;
}

