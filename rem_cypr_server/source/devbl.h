
#ifndef DEVBL_H
#define DEVBL_H

#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>

#define SIZE_BUF           4096
#define CMD_USBIP_DEV_LIST "/usr/sbin/usbip list -l | grep busid"
#define CMD_CYPRS_DEV_LIST "lsusb | grep Cypress"

#pragma pack(push,1)
typedef struct _USBIP_DEV_LIST_ENTRY
{
    unsigned char path[6];
    unsigned char id[10];
    unsigned char bind;
} USBIP_DEV_LIST_ENTRY, *PUSBIP_DEV_LIST_ENTRY;
#pragma pack(pop)

using namespace std;

int get_dev_boards_list(vector<USBIP_DEV_LIST_ENTRY> &devs);

#endif /* DEVBL_H */

