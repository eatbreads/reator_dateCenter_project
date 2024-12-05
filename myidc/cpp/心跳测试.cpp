#include "_public.h"
using namespace idc;
using namespace std;

clogfile logfile;

cpactive pactive;
int main(int argc, char *argv[])
{
    pactive.addpinfo(10);

    while(1){
        sleep(20);
    pactive.uptatime();
    }return 0;
}
