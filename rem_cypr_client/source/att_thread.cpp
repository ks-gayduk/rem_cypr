
#include "att_thread.h"

extern char ip_serv[20];
extern char sysfs_path[20];

void AttachThread::run()
{
    char buf[50];
    sprintf(buf, "usbip.exe -a %s %s", ip_serv, sysfs_path);
    system(buf);
}
