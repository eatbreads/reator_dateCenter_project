# 此脚本用于启动数据共享平台全部的服务程序。

# 启动守护模块，建议在/etc/rc.local中配置，以超级用户的身份启动。
/project/myidc/bin/procctl 10 /project/myidc/bin/守护进程 /tmp/log/checkproc.log

# 生成气象站点观测的分钟数据，程序每分钟运行一次。
/project/myidc/bin/procctl 60 /project/myidc/bin/crtsurfdata /project/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml

#执行清理数据的能力
/project/myidc/bin/procctl 3600 /project/myidc/bin/清理程序 /tmp/idc/surfdata "*.xml,*.json,*csv" 0.1

#执行站点参数入库的能力,一小时运行一次
/project/myidc/bin/procctl 3600 /project/myidc/db/参数入库 /project/idc/ini/stcode.ini "idc/idcpwd" "Simplified Chinese_China.AL32UTF8" /log/idc/obtcodetodb.log

#执行观测数据入库 ,3小时运行一个
/project/myidc/bin/procctl 10000 /project/myidc/db/观测数据入库 /tmp/idc/surfdata "idc/idcpwd" "Simplified Chinese_China.AL32UTF8" /log/idc/obtmindtodb.log